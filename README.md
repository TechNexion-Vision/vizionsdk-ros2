# vizionsdk_ros2

C++ ROS2 wrapper for VizionSDK cameras. The node opens a VizionSDK camera, publishes IMU data by default, and can optionally publish images, camera calibration, camera status, and basic ISP controls.

## Prerequisites

Install ROS2 first. Replace `<distro>` with your ROS2 distribution, for example `humble` or `jazzy`.

```bash
source /opt/ros/<distro>/setup.bash
```

Install VizionSDK before building this wrapper. The VizionSDK `.deb` or apt package must provide:

- VizionSDK headers
- `libVizionSDK.so`
- `vizionsdkConfig.cmake` or `vizionsdk.pc`

Install ROS2 build dependencies:

```bash
sudo apt update
sudo apt install \
  build-essential \
  cmake \
  pkg-config \
  python3-colcon-common-extensions \
  ros-<distro>-ament-cmake \
  ros-<distro>-diagnostic-msgs \
  ros-<distro>-rcl-interfaces \
  ros-<distro>-rclcpp \
  ros-<distro>-sensor-msgs
```

Install RViz if you want to view the IMU and camera topics:

```bash
sudo apt install ros-<distro>-rviz2
```

Install image viewing tools and the compressed image transport plugin if you want
to view `MJPG` streams:

```bash
sudo apt install \
  ros-<distro>-rqt-image-view \
  ros-<distro>-compressed-image-transport
```

## Build From Source

Clone this wrapper into a ROS2 workspace:

```bash
mkdir -p ~/ros2_ws/src
cd ~/ros2_ws/src
git clone <vizionsdk-ros2-wrapper-repo-url> vizionsdk_ros2
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

IMU only:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py
```

Select a camera by list index:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py device_index:=0
```

IMU plus camera stream:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py \
  device_index:=0 \
  publish_image:=true \
  image_format:=MJPG
```

Use RViz:

```bash
rviz2 -d install/vizionsdk_ros2/share/vizionsdk_ros2/rviz/vizionsdk_imu_camera_compressed.rviz
```

View the `MJPG` stream with `rqt_image_view`:

```bash
rqt_image_view
```

Select the `image_raw/compressed` topic in the viewer.

If RViz or `rqt_image_view` reports an error like this:

```text
Error subscribing: Unable to load plugin for transport 'image_transport/compressed_sub'
Declared types are image_transport/raw_sub
```

the computer running the viewer is missing the compressed image transport
subscriber plugin. Install it with:

```bash
sudo apt install ros-<distro>-compressed-image-transport
```

Then source the ROS2 environment again and restart the viewer:

```bash
source /opt/ros/<distro>/setup.bash
source install/local_setup.bash
```

If RViz reports `unsupported image encoding uyvy`, the selected camera format is
a packed YUV format that RViz cannot display directly. Use `MJPG` with the
compressed image transport plugin, run the converter node below, or select an
RViz-friendly raw format such as `BGR`, `BGRA`, or `RGB` if the camera
advertises one:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py \
  publish_image:=true \
  image_format:=BGR
```

Raw RGB/BGR streams use much more bandwidth than `MJPG`.

To keep the camera stream as `UYVY` for customers while also viewing it in RViz,
start the camera with `UYVY`:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py \
  publish_imu:=false \
  publish_image:=true \
  image_format:=UYVY
```

Then start the converter in another terminal:

```bash
ros2 run vizionsdk_ros2 yuv422_to_bgr_node --ros-args \
  -p input_topic:=/image_raw \
  -p output_topic:=/image_raw/bgr
```

Subscribe RViz or `rqt_image_view` to `image_raw/bgr`. The original
`image_raw` topic remains `uyvy` for applications that need the camera's raw
format.

## ISP Controls

Set ISP exposure controls at launch:

```bash
ros2 launch vizionsdk_ros2 vizionsdk_camera.launch.py \
  isp_exposure_mode:=0 \
  isp_exposure_time:=8000 \
  isp_exposure_gain:=4
```

Set an ISP control while the node is running:

```bash
ros2 param set /vizionsdk_camera isp.exposure_time 8000
```

Read current exposure and gain status:

```bash
ros2 topic echo /camera/status
```

## Topics

- `imu/data`: `sensor_msgs/msg/Imu`
- `image_raw`: `sensor_msgs/msg/Image` for uncompressed formats
- `image_raw/compressed`: `sensor_msgs/msg/CompressedImage` for `MJPG`
- `camera_info`: `sensor_msgs/msg/CameraInfo` when `VxGetIntrinsics` succeeds
- `camera/status`: `diagnostic_msgs/msg/DiagnosticArray` with current exposure, gain, and ISP ranges

## Key Parameters

- `device_index`: VizionSDK camera index, default `0`
- `publish_imu`: enable IMU publishing, default `true`
- `publish_status`: enable camera status publishing, default `true`
- `status_rate_hz`: camera status publishing rate, default `1.0`
- `imu_rate_hz`: IMU polling rate, default `100.0`
- `imu_mode`: `ispu`, `self_test`, or `disable`, default `ispu`
- `acceleration_unit`: SDK acceleration unit before ROS conversion, `mg`, `g`, `m/s^2`, or `raw`, default `mg`
- `gyro_unit`: SDK gyro unit before ROS conversion, `deg/s`, `rad/s`, or `raw`, default `deg/s`
- `publish_image`: enable image publishing, default `false`
- `image_format`: `auto`, `YUY2`, `UYVY`, `NV12`, `MJPG`, `BGRA`, `BGRX`, `BGR`, `RGB16`, or `RGB`
- `image_width`, `image_height`, `image_framerate`: `0` means wildcard
- `image_rate_hz`: `0.0` uses the selected camera format frame rate
- `isp.*`: ISP image processing controls, `-1` leaves the SDK value unchanged
- `isp.exposure_mode`, `isp.exposure_time`, `isp.exposure_gain`: exposed as launch arguments
