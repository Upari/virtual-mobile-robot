/*
---已弃用, 功能已被 Robot_controller 代替
但是有学习价值, 可以学习如何控制小车
virtual_diff_drive_node  驾驶节点

读取 /cmd_vel 上的消息, 同时判断是否超时
*/
#include <memory>
#include <chrono>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
// Cmd_Vel msg
#include "geometry_msgs/msg/twist.hpp"
// Odometry
#include "nav_msgs/msg/odometry.hpp"
// Quaternion
#include "tf2/LinearMath/Quaternion.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
// Dynamic boradcaster
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_broadcaster.hpp"

using Twist = geometry_msgs::msg::Twist;
using Odometry = nav_msgs::msg::Odometry;
using namespace std::chrono_literals;

class VirtualDiffDriveNode : public rclcpp::Node
{
public:
    VirtualDiffDriveNode()
    : Node("virtual_diff_drive_node"), 
      linear_velocity_(0.0), 
      angular_velocity_(0.0),
      x_(0.0), y_(0.0), yaw_(0.0),
      timer_run_period_(50ms),
      last_update_time_(this->now()),
      last_cmd_vel_time_(this->now()),
      cmd_vel_timeout_(0.5),
      if_cmd_vel_timeout_(false)
    {
        // 订阅 /cmd_vel 上的消息, 同时把目标速度值写入私有变量
        subscription_ = this->create_subscription<Twist>(
            "cmd_vel",
            10,
            [this](const Twist::SharedPtr msg)
            {
                this->cmd_vel_callback(msg);
            }
        );

        // 配合下面的 tf_broadcaster 发布 TF : 
        odom_publisher_ = this->create_publisher<Odometry>(
            "odom",
            10
        );
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        // 定时任务
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
    void check_cmd_vel_timeout(const rclcpp::Time & current_time)
    {
        const double elapsed = (current_time - last_cmd_vel_time_).seconds();
        if (elapsed > cmd_vel_timeout_) 
        {
            linear_velocity_ = 0.0;
            angular_velocity_ = 0.0;

            if (!if_cmd_vel_timeout_)
            {
                RCLCPP_WARN(
                    this->get_logger(),
                    "Cmd_vel timeout after %.3f seconds, robot stopped.",
                    cmd_vel_timeout_
                );
                if_cmd_vel_timeout_ = true;
            }
        }

        
    }

    void cmd_vel_callback(const Twist::SharedPtr msg)
    {
        // The subscription callback only updates the latest velocity command.
        // Other periodic logic is handled in the timer callback
        linear_velocity_ = msg->linear.x;       
        angular_velocity_ = msg->angular.z;
        last_cmd_vel_time_ = this->now();
        if_cmd_vel_timeout_ = false;

        RCLCPP_INFO(
            this->get_logger(),
            "Received new cmd_vel."
        );
    }

    void update_pose(const double dt)
    {
        // calculate odom
        x_ += linear_velocity_ * std::cos(yaw_) * dt;
        y_ += linear_velocity_ * std::sin(yaw_) * dt;
        yaw_ += angular_velocity_ * dt;

        // timer_callback period
        RCLCPP_INFO_THROTTLE(
            this->get_logger(),
            *this->get_clock(),
            1000,
            "Current velocity: linear.x = %.3f m/s, angular.z = %.3f rad/s |"
            "Pose: x = %.3f, y = %.3f, yaw = %.3f rad |"
            "timer_callback_period: %.3fs",
            linear_velocity_,
            angular_velocity_,
            x_, y_, yaw_,
            dt
        );
    }

    void publish_odom(const rclcpp::Time & stamp)
    {
        Odometry odom_msg;

        odom_msg.header.stamp = stamp;
        odom_msg.header.frame_id = "odom";
        odom_msg.child_frame_id = "base_link";

        odom_msg.pose.pose.position.x = x_;
        odom_msg.pose.pose.position.y = y_;
        odom_msg.pose.pose.position.z = 0.0;

        tf2::Quaternion quaternion;
        quaternion.setRPY(
            0.0,    // roll
            0.0,    // pitch
            yaw_    // yaw
        );

        odom_msg.pose.pose.orientation = tf2::toMsg(quaternion);

        odom_msg.twist.twist.linear.x = linear_velocity_;
        odom_msg.twist.twist.angular.z = angular_velocity_;

        odom_publisher_->publish(odom_msg);

    }

    void broadcast_odom_tf(const rclcpp::Time & stamp)
    {
        geometry_msgs::msg::TransformStamped transform;

        transform.header.stamp = stamp;
        transform.header.frame_id = "odom";
        transform.child_frame_id = "base_link";

        transform.transform.translation.x = x_;
        transform.transform.translation.y = y_;
        transform.transform.translation.z = 0.0;

        tf2::Quaternion quaternion;
        quaternion.setRPY(
            0.0,
            0.0,
            yaw_
        );
        transform.transform.rotation = tf2::toMsg(quaternion);

        tf_broadcaster_->sendTransform(transform);
    }

    void timer_callback()
    {
        // timer_callback 函数使用 cmd_vel_callback 已存储的速度信息

        // calculate dt  (real dt)
        const rclcpp::Time current_time = this->now();
        const double dt = (current_time - last_update_time_).seconds();
        last_update_time_ = current_time;

        // 检查上一次 cmd_vel 是否执行超过规定时间, 若超过, 则设置速度为0
        // 否则小车会一直使用过时的 cmd_vel 一直跑
        check_cmd_vel_timeout(current_time);
        // 更新小车的位置, 通过速度对时间的积分计算路程
        update_pose(dt);
        // 发布 /odom 消息
        publish_odom(current_time);
        // 发布 TF: odom->base_link
        broadcast_odom_tf(current_time);
    }

    rclcpp::Subscription<Twist>::SharedPtr subscription_;
    rclcpp::TimerBase::SharedPtr timer_;
    double linear_velocity_;
    double angular_velocity_;
    double x_, y_, yaw_;
    std::chrono::milliseconds timer_run_period_;
    rclcpp::Time last_update_time_;

    rclcpp::Publisher<Odometry>::SharedPtr odom_publisher_;

    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

    rclcpp::Time last_cmd_vel_time_;
    double cmd_vel_timeout_;
    bool if_cmd_vel_timeout_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<VirtualDiffDriveNode>();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}