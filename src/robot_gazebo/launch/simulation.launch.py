import os       # 拼接文件地址

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
        Command(["xacro", " ", xacro_file]),
        value_type=str,
    )

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

    return LaunchDescription([
        gazebo,
        robot_state_publisher,
        bridge,
        delayed_spawn_robot,
    ])