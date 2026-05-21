from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    args = [
        DeclareLaunchArgument("node_name", default_value="vizionsdk_camera"),
        DeclareLaunchArgument("device_index", default_value="0"),
        DeclareLaunchArgument("publish_imu", default_value="true"),
        DeclareLaunchArgument("publish_status", default_value="true"),
        DeclareLaunchArgument("status_rate_hz", default_value="1.0"),
        DeclareLaunchArgument("imu_rate_hz", default_value="100.0"),
        DeclareLaunchArgument("imu_mode", default_value="ispu"),
        DeclareLaunchArgument("publish_ispu_orientation", default_value="true"),
        DeclareLaunchArgument("acceleration_unit", default_value="mg"),
        DeclareLaunchArgument("gyro_unit", default_value="deg/s"),
        DeclareLaunchArgument("imu_frame_id", default_value="vizion_imu_link"),
        DeclareLaunchArgument("publish_image", default_value="false"),
        DeclareLaunchArgument("image_rate_hz", default_value="0.0"),
        DeclareLaunchArgument("image_format", default_value="auto"),
        DeclareLaunchArgument("image_width", default_value="0"),
        DeclareLaunchArgument("image_height", default_value="0"),
        DeclareLaunchArgument("image_framerate", default_value="0"),
        DeclareLaunchArgument("image_timeout_ms", default_value="1000"),
        DeclareLaunchArgument("isp_exposure_mode", default_value="-1"),
        DeclareLaunchArgument("isp_exposure_time", default_value="-1"),
        DeclareLaunchArgument("isp_exposure_gain", default_value="-1"),
        DeclareLaunchArgument(
            "image_frame_id",
            default_value="vizionsdk_camera_optical_frame",
        ),
    ]

    node = Node(
        package="vizionsdk_ros2",
        executable="vizionsdk_camera_node",
        name=LaunchConfiguration("node_name"),
        output="screen",
        parameters=[
            {
                "device_index": ParameterValue(LaunchConfiguration("device_index"), value_type=int),
                "publish_imu": ParameterValue(LaunchConfiguration("publish_imu"), value_type=bool),
                "publish_status": ParameterValue(
                    LaunchConfiguration("publish_status"), value_type=bool
                ),
                "status_rate_hz": ParameterValue(
                    LaunchConfiguration("status_rate_hz"), value_type=float
                ),
                "imu_rate_hz": ParameterValue(LaunchConfiguration("imu_rate_hz"), value_type=float),
                "imu_mode": LaunchConfiguration("imu_mode"),
                "publish_ispu_orientation": ParameterValue(
                    LaunchConfiguration("publish_ispu_orientation"), value_type=bool
                ),
                "acceleration_unit": LaunchConfiguration("acceleration_unit"),
                "gyro_unit": LaunchConfiguration("gyro_unit"),
                "imu_frame_id": LaunchConfiguration("imu_frame_id"),
                "publish_image": ParameterValue(
                    LaunchConfiguration("publish_image"), value_type=bool
                ),
                "image_rate_hz": ParameterValue(
                    LaunchConfiguration("image_rate_hz"), value_type=float
                ),
                "image_format": LaunchConfiguration("image_format"),
                "image_width": ParameterValue(LaunchConfiguration("image_width"), value_type=int),
                "image_height": ParameterValue(
                    LaunchConfiguration("image_height"), value_type=int
                ),
                "image_framerate": ParameterValue(
                    LaunchConfiguration("image_framerate"), value_type=int
                ),
                "image_timeout_ms": ParameterValue(
                    LaunchConfiguration("image_timeout_ms"), value_type=int
                ),
                "isp.exposure_mode": ParameterValue(
                    LaunchConfiguration("isp_exposure_mode"), value_type=int
                ),
                "isp.exposure_time": ParameterValue(
                    LaunchConfiguration("isp_exposure_time"), value_type=int
                ),
                "isp.exposure_gain": ParameterValue(
                    LaunchConfiguration("isp_exposure_gain"), value_type=int
                ),
                "image_frame_id": LaunchConfiguration("image_frame_id"),
            }
        ],
    )

    return LaunchDescription(args + [node])
