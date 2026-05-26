from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


LAUNCH_ARGUMENTS = (
    ("node_name", "vizionsdk_camera", "ROS node name."),
    ("namespace", "", "ROS namespace for this camera."),
    ("device_index", "0", "VizionSDK camera index from list_devices."),
    ("publish_imu", "true", "Publish sensor_msgs/msg/Imu on imu/data."),
    ("publish_status", "true", "Publish diagnostic camera status on camera/status."),
    ("status_rate_hz", "1.0", "Camera status publishing rate in Hz."),
    ("imu_rate_hz", "100.0", "IMU polling and publishing rate in Hz."),
    ("imu_mode", "ispu", "SDK IMU mode: ispu, self_test, or disable."),
    (
        "imu_self_test_mode",
        "normal",
        "SDK IMU self-test mode: normal, positive, negative, or not_allowed.",
    ),
    ("publish_ispu_orientation", "true", "Use ISPU orientation data when available."),
    ("acceleration_unit", "mg", "SDK acceleration unit: mg, g, m/s^2, or raw."),
    ("gyro_unit", "deg/s", "SDK gyro unit: deg/s, rad/s, or raw."),
    ("imu_frame_id", "vizion_imu_link", "Frame ID for IMU messages."),
    ("publish_image", "false", "Publish camera image data."),
    ("image_rate_hz", "0.0", "Image publishing rate in Hz; 0 uses selected format FPS."),
    (
        "image_format",
        "auto",
        "Requested image format: auto, YUY2, UYVY, NV12, MJPG, BGRA, BGRX, BGR, RGB16, or RGB.",
    ),
    ("image_width", "0", "Requested image width; 0 accepts any width."),
    ("image_height", "0", "Requested image height; 0 accepts any height."),
    ("image_framerate", "0", "Requested image frame rate; 0 accepts any frame rate."),
    ("image_timeout_ms", "1000", "Image capture timeout in milliseconds."),
    ("image_frame_id", "vizionsdk_camera_optical_frame", "Frame ID for image and camera_info."),
)


ISP_CONTROL_PARAMETERS = (
    "isp.brightness",
    "isp.contrast",
    "isp.saturation",
    "isp.whitebalance_mode",
    "isp.whitebalance_temperature",
    "isp.exposure_mode",
    "isp.exposure_time",
    "isp.exposure_min_time",
    "isp.exposure_max_time",
    "isp.exposure_gain",
    "isp.gamma",
    "isp.sharpness",
    "isp.backlight_compensation",
    "isp.special_effect_mode",
    "isp.denoise",
    "isp.flip_mode",
    "isp.pan",
    "isp.tilt",
    "isp.zoom",
    "isp.flick_mode",
    "isp.jpeg_quality",
    "isp.trigger_mode",
    "isp.ehdr_mode",
    "isp.ehdr_exposure_min_number",
    "isp.ehdr_exposure_max_number",
    "isp.ehdr_ratio_min",
    "isp.ehdr_ratio_max",
    "isp.ehdr_atm_weight",
    "isp.ehdr_atm_max",
)


def isp_launch_argument_name(parameter_name):
    return parameter_name.replace(".", "_")


def generate_launch_description():
    args = [
        *[
            DeclareLaunchArgument(name, default_value=default_value, description=description)
            for name, default_value, description in LAUNCH_ARGUMENTS
        ],
        *[
            DeclareLaunchArgument(
                isp_launch_argument_name(parameter_name),
                default_value="-1",
                description=(
                    f"Initial {parameter_name} value; -1 leaves the current SDK value unchanged."
                ),
            )
            for parameter_name in ISP_CONTROL_PARAMETERS
        ],
    ]

    node = Node(
        package="vizionsdk_ros2",
        executable="vizionsdk_camera_node",
        name=LaunchConfiguration("node_name"),
        namespace=LaunchConfiguration("namespace"),
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
                "imu_self_test_mode": LaunchConfiguration("imu_self_test_mode"),
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
                "image_frame_id": LaunchConfiguration("image_frame_id"),
                **{
                    parameter_name: ParameterValue(
                        LaunchConfiguration(isp_launch_argument_name(parameter_name)),
                        value_type=int,
                    )
                    for parameter_name in ISP_CONTROL_PARAMETERS
                },
            }
        ],
    )

    return LaunchDescription(args + [node])
