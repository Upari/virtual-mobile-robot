"""
simulation.launch.py
仿真启动文件
启动:
    gazebo,                         仿真
    robot_state_publisher,          小车形状发布
    bridge,                         Gazebo ROS2 桥
    delayed_spawn_robot,            延迟生成小车在Gazebo里面
    rviz_node,                      启动Rviz
    delayed_spawn_ros_controllers,  延迟生成Ros Controller
"""

import os

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    robot_description_share = get_package_share_directory(
        "robot_description"
    )
    robot_gazebo_share = get_package_share_directory(
        "robot_gazebo"
    )
    controller_file = os.path.join(
        robot_gazebo_share, 
        "config", 
        "robot_controllers.yaml"
    )
    ros_gz_sim_share = get_package_share_directory(
        "ros_gz_sim"
    )

    xacro_file = os.path.join(
        robot_description_share,
        "urdf",
        "robot.urdf.xacro",
    )

    bridge_config_file = os.path.join(
        robot_gazebo_share,
        "config",
        "bridge.yaml",
    )

    world_file = os.path.join(
        robot_gazebo_share,
        "worlds",
        "lidar_world.sdf",
    )

    robot_description = ParameterValue(
        Command(["xacro", " ", xacro_file, " ",
                "controllers_file:=",
                controller_file
        ]),
        value_type=str,
    )

    # 仿真程序
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                ros_gz_sim_share,
                "launch",
                "gz_sim.launch.py",
            )
        ),
        launch_arguments={
            "gz_args": f"-r {world_file}",
        }.items(),
    )

    # 发布小车的形状
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[
            {
                "robot_description": robot_description,
                "use_sim_time": True,
            }
        ],
    )

    # Gazebo ROS2 桥, 从 Gazebo 发送 /scan /clock 到 ROS2 
    bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        name="ros_gz_bridge",
        output="screen",
        parameters=[
            {
                "config_file": bridge_config_file,
                "use_sim_time": True,
            }
        ],
    )

    # 生成小车, 后面可以把这个参数也写进yaml文件
    spawn_robot = Node(
        package="ros_gz_sim",
        executable="create",
        name="spawn_virtual_mobile_robot",
        output="screen",
        arguments=[
            "-world",
            "lidar_world",
            "-name",
            "virtual_mobile_robot",
            "-topic",
            "robot_description",
            "-x",
            "0.0",
            "-y",
            "0.0",
            "-z",
            "0.01",
        ],
    )
    delayed_spawn_robot = TimerAction(
        period=3.0,
        actions=[spawn_robot],
    )

    # Rviz
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        parameters=[
            {
                "use_sim_time": True,
            }
        ],
    )


    # ros2 controllers
    spawn_joint_state_broadcaster = Node(
        package="controller_manager",
        executable="spawner",
        name="spawn_joint_state_broadcaster",
        output="screen",
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager"
        ]
    )
    spawn_diff_drive_controller = Node(
        package="controller_manager",
        executable="spawner",
        name="spawn_diff_drive_controller",
        output="screen",
        arguments=[
            "diff_drive_controller",
            "--controller-manager",
            "/controller_manager",
            "--controller-ros-args",
            "--ros-args --remap /diff_drive_controller/cmd_vel:=/cmd_vel"
        ]
    )
    delayed_spawn_ros_controllers = TimerAction(
        period=3.0,
        actions=[
            spawn_joint_state_broadcaster,
            spawn_diff_drive_controller
            ]
    )


    return LaunchDescription([
        gazebo,
        robot_state_publisher,
        bridge,
        delayed_spawn_robot,
        rviz_node,
        delayed_spawn_ros_controllers,
    ])