"""
navigation.launch.py
导航启动文件
启动: 
    map_server,             加载地图
    bt_navigator,           
    planner_server,
    controller_server,
    behavior_server,
    lifecycle_manager,      管理生命周期
    amcl,                   AMCL
"""

from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    params_file = "/home/user/virtual-mobile-robot/src/robot_gazebo/config/nav2_params.yaml"

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

    # AMCL
    amcl = Node(
        package="nav2_amcl",
        executable="amcl",
        name="amcl",
        output="screen",
        parameters=[params_file],
    )

    # RETURN 
    return LaunchDescription([
        map_server,
        bt_navigator,
        planner_server,
        controller_server,
        behavior_server,
        lifecycle_manager,
        amcl,
    ])
