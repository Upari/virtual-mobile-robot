#include <memory>
#include <chrono>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using Twist = geometry_msgs::msg::Twist;
using namespace std::chrono_literals;

class VirtualDiffDriveNode : public rclcpp::Node
{
public:
    VirtualDiffDriveNode()
    : Node("virtual_diff_drive_node"), 
      linear_velocity_(0.0), 
      angular_velocity_(0.0),
      x_(0.0), y_(0.0), yaw_(0.0),
      timer_run_period_(500ms),
      last_update_time_(this->now())
    {
        subscription_ = this->create_subscription<Twist>(
            "cmd_vel",
            10,
            [this](const Twist::SharedPtr msg)
            {
                this->cmd_vel_callback(msg);
            }
        );

        timer_ = this->create_wall_timer(
            timer_run_period_,
            [this]()
            {
                this->timer_callback();
            }
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Virtual diff drive node started."
        );
    }
private:
    void cmd_vel_callback(const Twist::SharedPtr msg)
    {
        // The subscription callback only updates the latest velocity command.
        // Other periodic logic is handled in the timer callback
        linear_velocity_ = msg->linear.x;       
        angular_velocity_ = msg->angular.z;

        RCLCPP_INFO(
            this->get_logger(),
            "Received new cmd_vel."
        );
    }

    void timer_callback()
    {
        // The timer callback period reads and uses the stored velocity.
        
        // calculate dt  (real dt)
        const rclcpp::Time current_time = this->now();
        const double dt = (current_time - last_update_time_).seconds();
        last_update_time_ = current_time;

        // calculate odom
        x_ += linear_velocity_ * std::cos(yaw_) * dt;
        y_ += linear_velocity_ * std::sin(yaw_) * dt;
        yaw_ += angular_velocity_ * dt;

        // timer_callback period
        RCLCPP_INFO(
            this->get_logger(),
            "Current velocity: linear.x = %.3f m/s, angular.z = %.3f rad/s"
            "Pose: x = %.3f, y = %.3f, yaw = %.3f rad"
            "timer_callback_period: %.3fs",
            linear_velocity_,
            angular_velocity_,
            x_, y_, yaw_,
            dt
        );
    }

    rclcpp::Subscription<Twist>::SharedPtr subscription_;
    rclcpp::TimerBase::SharedPtr timer_;
    double linear_velocity_;
    double angular_velocity_;
    double x_, y_, yaw_;
    std::chrono::milliseconds timer_run_period_;
    rclcpp::Time last_update_time_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<VirtualDiffDriveNode>();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}