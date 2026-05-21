#pragma once

#include "VizionSDK.h"
#include "vizionsdk_ros2/conversions.hpp"
#include "vizionsdk_ros2/isp_controls.hpp"

#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <rcl_interfaces/msg/set_parameters_result.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/imu.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace vizionsdk_ros2 {

class VizionSdkCameraNode final : public rclcpp::Node {
public:
    explicit VizionSdkCameraNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~VizionSdkCameraNode() override;

private:
    void DeclareAndReadParameters();
    void OpenCamera();
    void CloseCamera();
    void ConfigureImu();
    void ConfigureImage();
    void ConfigureIspControls();
    void ConfigureStatus();
    void LoadCameraInfo();

    void PublishImu();
    void PublishImage();
    void PublishCameraInfo(const rclcpp::Time& stamp);
    void PublishStatus();
    rcl_interfaces::msg::SetParametersResult OnSetParameters(
        const std::vector<rclcpp::Parameter>& parameters);
    bool ApplyIspControl(VX_ISP_IMAGE_PROPERTIES property, int value, std::string& error);

    VX_IMU_MODE ParseImuModeParameter(const std::string& value) const;
    VX_IMU_SELF_TEST_MODE ParseSelfTestModeParameter(const std::string& value) const;
    uint16_t ReadUint16Parameter(const std::string& name, int default_value);

    int device_index_{0};
    bool publish_imu_{true};
    bool publish_ispu_orientation_{true};
    bool publish_image_{false};
    bool publish_status_{true};
    double imu_rate_hz_{100.0};
    double image_rate_hz_{0.0};
    double status_rate_hz_{1.0};
    int image_timeout_ms_{1000};
    std::string imu_frame_id_{"vizion_imu_link"};
    std::string image_frame_id_{"vizionsdk_camera_optical_frame"};

    VX_IMU_MODE imu_mode_{VX_IMU_MODE::ISPU};
    VX_IMU_SELF_TEST_MODE self_test_mode_{VX_IMU_SELF_TEST_MODE::NORMAL};
    AccelerationUnit acceleration_unit_{AccelerationUnit::MilliG};
    GyroUnit gyro_unit_{GyroUnit::DegreesPerSecond};
    ImageFormatRequest image_request_{};

    std::shared_ptr<VxCamera> camera_;
    bool streaming_started_{false};
    VxFormat image_format_{};
    std::vector<uint8_t> image_buffer_;
    std::optional<sensor_msgs::msg::CameraInfo> camera_info_;

    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr compressed_image_pub_;
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_pub_;
    rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr status_pub_;

    rclcpp::TimerBase::SharedPtr imu_timer_;
    rclcpp::TimerBase::SharedPtr image_timer_;
    rclcpp::TimerBase::SharedPtr status_timer_;
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr parameter_callback_;
};

}  // namespace vizionsdk_ros2
