// Twist (Nav2)  --> StampedTwist (cmd_vel --> diff_drive_controller)
#include <memory>
#include <utility>

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>

using Twist = geometry_msgs::msg::Twist;
using TwistStamped = geometry_msgs::msg::TwistStamped;

class CmdVelBridge : public rclcpp::Node
{
public:
    CmdVelBridge() : Node("cmd_vel_bridge")
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Start cmd_vel bridge."
        );

        pub_ = this->create_publisher<TwistStamped>("/cmd_vel", 10);

        // core logical
        sub_ = this->create_subscription<Twist>(
            "/cmd_vel_nav", 10,
            [this](const Twist::SharedPtr msg)
            {
                auto out = std::make_unique<TwistStamped>();
                out->header.stamp = this->now();
                out->header.frame_id = "base_link";
                out->twist = *msg;
                pub_->publish(std::move(out));
            }
        );
    }
private:
    rclcpp::Publisher<TwistStamped>::SharedPtr pub_;
    rclcpp::Subscription<Twist>::SharedPtr sub_;
};


int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CmdVelBridge>());
  rclcpp::shutdown();
  return 0;
}