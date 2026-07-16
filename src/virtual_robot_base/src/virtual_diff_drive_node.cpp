#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using Twist = geometry_msgs::msg::Twist;

class VirtualDiffDriveNode : public rclcpp::Node
{
public:
    VirtualDiffDriveNode()
    : Node("virtual_diff_drive_node")
    {
        subscription_ = this->create_subscription<Twist>(
            "cmd_vel",
            10,
            [this](const Twist msg)
            {
                this->cmd_vel_callback(msg);
            }
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Virtual_diff_drive_node Started."
        );
    }
private:
    void cmd_vel_callback(const Twist msg)
    {
        double linear_x = msg.linear.x;       
        double angular_z = msg.angular.z;

        RCLCPP_INFO(
            this->get_logger(),
            "Received cmd_vel: linear.x = %.3f m/s, angular.z = %.3f rad/s",
            linear_x, angular_z
        );
    }

    rclcpp::Subscription<Twist>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<VirtualDiffDriveNode>();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}