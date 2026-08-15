"""
SLAM 建图 启动文件

"""

import os

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    robot_gazebo_share = get_package_share_directory("robot_gazebo")
    slam_toolbox_share = get_package_share_directory("slam_toolbox")

    slam_params_file = os.path.join(robot_gazebo_share, "config", "slam_params.yaml")
    rviz_config_file = os.path.join(robot_gazebo_share, "config", "rviz_config.rviz")

    # 官方 launch：autostart=true 自动过 configure → activate（不用手动 lifecycle）
    slam = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(slam_toolbox_share, "launch", "online_sync_launch.py")
        ),
        launch_arguments={
            "slam_params_file": slam_params_file,
            "use_sim_time": "true",
        }.items(),
    )

    amcl = Node(
        package="nav2_amcl",
        executable="amcl",
        name="amcl",
        output="screen",
        parameters=[slam_params_file],
    )

    return LaunchDescription([
        slam, 
        amcl,
    ])