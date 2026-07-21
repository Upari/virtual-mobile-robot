#include <memory>
#include <chrono>
#include <cmath>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

using LaserScan = sensor_msgs::msg::LaserScan;
using namespace std::chrono_literals;

class VirtualLidarNode : public rclcpp::Node
{
public:
    VirtualLidarNode()
    : Node("virtual_lidar_node"),
    angle_min_(-M_PI),
    angle_max_(M_PI),
    angle_increment_(M_PI / 180.0),
    range_min_(0.1),
    range_max_(10.0),
    fixed_range_(2.0)
    {
        scan_publisher_ = this->create_publisher<LaserScan>(
            "scan",
            10
        );

        timer_ = this->create_wall_timer(
            100ms,
            [this]()
            {
                this->publish_scan();
            }
        );

        RCLCPP_INFO(
            this->get_logger(),
            "scan node started."
        );
    }
private:
    void publish_scan()
    {
        LaserScan scan_msg;

        scan_msg.header.stamp = this->now();
        scan_msg.header.frame_id = "laser_frame";

        scan_msg.angle_min = angle_min_;
        scan_msg.angle_max = angle_max_;

        scan_msg.angle_increment = angle_increment_;
        scan_msg.scan_time = 0.1;

        scan_msg.range_min = range_min_;
        scan_msg.range_max = range_max_;

        const std::size_t beam_count = 
            static_cast<std::size_t>(
                std::round((angle_max_ - angle_min_) / angle_increment_) + 1
            );

        scan_msg.ranges.assign(
            beam_count,
            static_cast<float>(fixed_range_)
        );

        // add obstacle
        const double obstacle_angle_min = -10.0 * M_PI / 180.0;
        const double obstacle_angle_max = 10.0 * M_PI / 180.0;

        for (std::size_t i = 0; i < beam_count; ++i)
        {
            const double angle = angle_min_ + static_cast<double>(i) * angle_increment_;
            if (angle >= obstacle_angle_min && angle <= obstacle_angle_max)
            {
                scan_msg.ranges[i] = 1.0f;
            }

        }

        scan_publisher_->publish(scan_msg);
    }

    rclcpp::Publisher<LaserScan>::SharedPtr scan_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    double angle_min_;
    double angle_max_;
    double angle_increment_;
    double range_min_;
    double range_max_;
    double fixed_range_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<VirtualLidarNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    
    return 0;
}