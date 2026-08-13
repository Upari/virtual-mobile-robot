import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    params_file = "/home/user/virtual-mobile-robot/src/robot_gazebo/config/nav2_params.yaml"

    # map -> odom
    static_map_odom = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="static_map_odom",
        arguments=["0", "0", "0", "0", "0", "0", "map", "odom"],
    )

    # map_server : load map, publish /map
    map_server = Node(
        package="nav2_map_server",
        executable="map_server",
        name="map_server", 
        output="screen",
        parameters=[params_file],
    )

    # nav2 node
    bt_navigator = Node(
        package="nav2_bt_navigator",
        executable="bt_navigator",
        name="bt_navigator",
        output="screen",
        parameters=[params_file],
    )
    planner_server = Node(
        package="nav2_planner",
        executable="planner_server",
        name="planner_server",
        output="screen",
        parameters=[params_file],
    )
    controller_server = Node(
        package="nav2_controller",
        executable="controller_server",
        name="controller_server",
        output="screen",
        parameters=[params_file],
    )
    behavior_server = Node(
        package="nav2_behaviors",
        executable="behavior_server",
        name="behavior_server",
        output="screen",
        parameters=[params_file],
    )

    # lifecycle manager
    lifecycle_manager = Node(
        package="nav2_lifecycle_manager",
        executable="lifecycle_manager",
        name="lifecycle_manager_navigation",
        output="screen",
        parameters=[params_file],
    )

    # Twist (Nav2)  --> StampedTwist (cmd_vel --> diff_drive_controller)
    cmd_vel_bridge = Node(
        package="robot_gazebo",
        executable="cmd_vel_bridge",
        name="cmd_vel_bridge",
        output="screen",
        parameters=[
            {"use_sim_time": True}
        ],
    )

    # RETURN 
    return LaunchDescription([
        static_map_odom,
        map_server,
        bt_navigator,
        planner_server,
        controller_server,
        behavior_server,
        lifecycle_manager,
        cmd_vel_bridge,
    ])
