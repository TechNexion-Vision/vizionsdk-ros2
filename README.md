# vizionsdk_ros2

C++ ROS 2 wrapper for VizionSDK cameras. The package opens a VizionSDK camera,
publishes IMU data by default, and can optionally publish images, camera
calibration, camera status, and ISP controls.

## What is included in the repository

- `src/` and `include/`: ROS 2 camera node, device-list utility, and conversion helpers.
- `launch/`: launch file for the VizionSDK camera node.
- `rviz/`: RViz configurations for IMU and image viewing.
- `test/`: unit tests for conversion helpers, device-list formatting, ISP controls, and launch coverage.

## Supported Platforms

This wrapper targets Linux platforms where both ROS 2 and VizionSDK are available.

| Platform | Architecture | ROS 2 Distribution |
|-----------|---------------|--------------------|
| Ubuntu 22.04 | AMD64 / ARM64 | Humble |
| Ubuntu 24.04 | AMD64 / ARM64 | Jazzy |
| Ubuntu 26.04 | AMD64 / ARM64 | Lyrical |
| NVIDIA Jetson / NXP i.MX / Raspberry Pi / TI TDA4VM | ARM64 | Distribution-dependent |

Camera support follows the installed VizionSDK release. See the VizionSDK
documentation for the complete platform, camera, and sensor support matrix.

## Prerequisites

Install ROS 2 and VizionSDK before building this wrapper. Replace `<distro>`
with the ROS 2 distribution for the installed Ubuntu release:

| Ubuntu | `<distro>` |
|--------|------------|
| 22.04 | `humble` |
| 24.04 | `jazzy` |
| 26.04 | `lyrical` |

```bash
source /opt/ros/<distro>/setup.bash
```

Install ROS 2 build dependencies:

```bash
sudo apt update
sudo apt install \
  build-essential \
  cmake \
  pkg-config \
  python3-colcon-common-extensions \
  ros-<distro>-ament-cmake \
  ros-<distro>-diagnostic-msgs \
  ros-<distro>-launch \
  ros-<distro>-launch-ros \
  ros-<distro>-rcl-interfaces \
  ros-<distro>-rclcpp \
  ros-<distro>-sensor-msgs
```

## Build From Source

Clone this wrapper into a ROS 2 workspace:

```bash
mkdir -p ~/ros2_ws/src
cd ~/ros2_ws/src
git clone https://github.com/TechNexion-Vision/vizionsdk-ros2-wrapper.git vizionsdk_ros2
cd ~/ros2_ws
```

Install ROS dependencies and build:

```bash
source /opt/ros/<distro>/setup.bash
rosdep update
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install \
  --packages-select vizionsdk_ros2 \
  --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/local_setup.bash
```

If VizionSDK was installed to a custom prefix, expose it before building:

```bash
export CMAKE_PREFIX_PATH=/opt/vizionsdk:$CMAKE_PREFIX_PATH
export PKG_CONFIG_PATH=/opt/vizionsdk/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=/opt/vizionsdk/lib:$LD_LIBRARY_PATH
```

## Run

List connected cameras:

```bash
ros2 run vizionsdk_ros2 list_devices
```

List connected cameras with supported image formats:

```bash
ros2 run vizionsdk_ros2 list_devices --formats
```

Start the camera node with IMU publishing:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py
```

Start IMU plus camera image publishing:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py \
  device_index:=0 \
  publish_image:=true \
  image_format:=MJPG \
  image_width:=1920 \
  image_height:=1080 \
  image_framerate:=30
```

Use `ros2 run vizionsdk_ros2 list_devices --formats` to see the formats,
resolutions, and frame rates advertised by each camera. Pass those values with
`image_format`, `image_width`, `image_height`, and `image_framerate`; leave a
value as `0` to accept any match for that field.

Start multiple cameras by launching one node per camera with separate ROS
namespaces. For example, start the first camera:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py \
  namespace:=cam0 \
  node_name:=vizionsdk_camera_cam0 \
  device_index:=0 \
  image_frame_id:=cam0_camera_optical_frame \
  imu_frame_id:=cam0_imu_link \
  publish_image:=true \
  image_format:=UYVY \
  image_width:=1280 \
  image_height:=720 \
  image_framerate:=30
```

Then start the second camera in another terminal:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py \
  namespace:=cam1 \
  node_name:=vizionsdk_camera_cam1 \
  device_index:=1 \
  image_frame_id:=cam1_camera_optical_frame \
  imu_frame_id:=cam1_imu_link \
  publish_image:=true \
  image_format:=UYVY \
  image_width:=1280 \
  image_height:=720 \
  image_framerate:=30
```

## Topics

- `imu/data`: `sensor_msgs/msg/Imu`
- `image_raw`: `sensor_msgs/msg/Image` for uncompressed formats
- `image_raw/compressed`: `sensor_msgs/msg/CompressedImage` for `MJPG`
- `camera_info`: `sensor_msgs/msg/CameraInfo` when `VxGetIntrinsics` succeeds
- `camera/status`: `diagnostic_msgs/msg/DiagnosticArray`

When a namespace is set, these topics are published below it. For example,
`namespace:=cam0` publishes `/cam0/image_raw`, `/cam0/imu/data`,
`/cam0/camera_info`, and `/cam0/camera/status`.

## Parameters

Show all launch arguments and descriptions:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py --show-args
```

See [docs/usage.md](docs/usage.md) for RViz viewing, YUV422 conversion, ISP
controls, IMU self-test, and test commands.

## Documentation

- [VizionSDK Overview](https://developer.technexion.com/docs/vision-software/vizionsdk/)
- [VizionSDK API User Guide](https://developer.technexion.com/docs/category/vizionsdk-api)
- [VizionSDK C++ Installation](https://developer.technexion.com/docs/vision-software/vizionsdk/cplusplus/vizionsdk-cpp-installation)

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
