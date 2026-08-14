
import os

from launch import LaunchDescription
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    robot_description_share = get_package_share_directory(
        "robot_description"
    )

    xacro_file = os.path.join(
        robot_description_share,
        "urdf",
        "robot.urdf.xacro"
    )

    rviz_config_file = os.path.join(
        robot_description_share,
        "rviz",
        "robot.rviz"
    )

    robot_description = ParameterValue(
        Command([
            "xacro",
            " ",
            xacro_file
        ]),
        value_type=str
    )

    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[
            {
                "robot_description": robot_description
            }
        ]
    )

    virtual_diff_drive_node = Node(
        package="virtual_robot_base",
        executable="virtual_diff_drive_node",
        name="virtual_diff_drive_node",
        output="screen"
    )

    vritual_lidar_node = Node(
        package="virtual_robot_sensors",
        executable="virtual_lidar_node",
        name="virtual_lidar_node",
        output="screen"
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        # argument=[
        #     "-d",
        #     rviz_config_file
        # ]
    )
    
    return LaunchDescription([
        robot_state_publisher_node,
        virtual_diff_drive_node,
        vritual_lidar_node,
        rviz_node
    ])