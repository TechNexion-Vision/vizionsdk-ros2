#include "vizionsdk_ros2/vizionsdk_camera_node.hpp"

#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <diagnostic_msgs/msg/key_value.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace vizionsdk_ros2 {
namespace {

template <typename Rep, typename Period>
std::chrono::nanoseconds ToNanoseconds(std::chrono::duration<Rep, Period> duration) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
}

std::string FormatRequestDescription(const ImageFormatRequest& request) {
    std::ostringstream oss;
    oss << "format=" << FormatName(request.format)
        << ", width=" << request.width
        << ", height=" << request.height
        << ", fps=" << request.framerate;
    return oss.str();
}

}  // namespace

VizionSdkCameraNode::VizionSdkCameraNode(const rclcpp::NodeOptions& options)
    : rclcpp::Node("vizionsdk_camera", options) {
    DeclareAndReadParameters();
    OpenCamera();
    ConfigureIspControls();
    ConfigureStatus();
    ConfigureImu();
    ConfigureImage();
}

VizionSdkCameraNode::~VizionSdkCameraNode() {
    CloseCamera();
}

void VizionSdkCameraNode::DeclareAndReadParameters() {
    device_index_ = this->declare_parameter<int>("device_index", 0);
    publish_imu_ = this->declare_parameter<bool>("publish_imu", true);
    publish_ispu_orientation_ = this->declare_parameter<bool>("publish_ispu_orientation", true);
    publish_image_ = this->declare_parameter<bool>("publish_image", false);
    publish_status_ = this->declare_parameter<bool>("publish_status", true);
    imu_rate_hz_ = this->declare_parameter<double>("imu_rate_hz", 100.0);
    image_rate_hz_ = this->declare_parameter<double>("image_rate_hz", 0.0);
    status_rate_hz_ = this->declare_parameter<double>("status_rate_hz", 1.0);
    image_timeout_ms_ = this->declare_parameter<int>("image_timeout_ms", 1000);
    imu_frame_id_ = this->declare_parameter<std::string>("imu_frame_id", "vizion_imu_link");
    image_frame_id_ =
        this->declare_parameter<std::string>("image_frame_id", "vizionsdk_camera_optical_frame");

    imu_mode_ = ParseImuModeParameter(this->declare_parameter<std::string>("imu_mode", "ispu"));
    self_test_mode_ = ParseSelfTestModeParameter(
        this->declare_parameter<std::string>("imu_self_test_mode", "normal"));

    const auto acceleration_unit = ParseAccelerationUnit(
        this->declare_parameter<std::string>("acceleration_unit", "mg"));
    if (!acceleration_unit.has_value()) {
        throw std::invalid_argument("acceleration_unit must be one of: mg, g, m/s^2, raw");
    }
    acceleration_unit_ = *acceleration_unit;

    const auto gyro_unit =
        ParseGyroUnit(this->declare_parameter<std::string>("gyro_unit", "deg/s"));
    if (!gyro_unit.has_value()) {
        throw std::invalid_argument("gyro_unit must be one of: deg/s, rad/s, raw");
    }
    gyro_unit_ = *gyro_unit;

    const auto requested_format =
        ParseImageFormat(this->declare_parameter<std::string>("image_format", "auto"));
    if (!requested_format.has_value()) {
        throw std::invalid_argument("image_format must be auto, YUY2, UYVY, NV12, MJPG, BGRA, "
                                    "BGRX, BGR, RGB16, or RGB");
    }
    image_request_.format = *requested_format;
    image_request_.width = ReadUint16Parameter("image_width", 0);
    image_request_.height = ReadUint16Parameter("image_height", 0);
    image_request_.framerate = ReadUint16Parameter("image_framerate", 0);

    for (const auto& parameter : IspControlParameters()) {
        this->declare_parameter<int>(parameter.parameter_name, -1);
    }

    if (device_index_ < 0) {
        throw std::invalid_argument("device_index must be >= 0");
    }
    if (imu_rate_hz_ <= 0.0) {
        throw std::invalid_argument("imu_rate_hz must be > 0");
    }
    if (image_rate_hz_ < 0.0) {
        throw std::invalid_argument("image_rate_hz must be >= 0");
    }
    if (status_rate_hz_ <= 0.0) {
        throw std::invalid_argument("status_rate_hz must be > 0");
    }
    if (image_timeout_ms_ <= 0) {
        throw std::invalid_argument("image_timeout_ms must be > 0");
    }
    if (image_timeout_ms_ > 65535) {
        throw std::invalid_argument("image_timeout_ms must be <= 65535");
    }
}

void VizionSdkCameraNode::OpenCamera() {
    std::vector<std::string> devices;
    const int count = VxDiscoverCameraDevices(devices);
    if (count <= 0 || static_cast<size_t>(device_index_) >= devices.size()) {
        throw std::runtime_error("No Vizion camera found for device_index " +
                                 std::to_string(device_index_));
    }

    camera_ = VxInitialCameraDevice(device_index_);
    if (!camera_) {
        throw std::runtime_error("VxInitialCameraDevice returned null");
    }

    if (VxOpen(camera_) != 0) {
        camera_.reset();
        throw std::runtime_error("VxOpen failed for device_index " + std::to_string(device_index_));
    }

    if (VxIsVizionCamera(camera_) != 0) {
        CloseCamera();
        throw std::runtime_error("Selected device is not a Vizion camera");
    }

    std::string device_name;
    if (VxGetDeviceName(camera_, device_name) == 0) {
        RCLCPP_INFO(this->get_logger(), "Opened Vizion camera: %s", device_name.c_str());
    } else {
        RCLCPP_INFO(this->get_logger(), "Opened Vizion camera at index %d", device_index_);
    }
}

void VizionSdkCameraNode::CloseCamera() {
    imu_timer_.reset();
    image_timer_.reset();
    status_timer_.reset();
    parameter_callback_.reset();

    if (camera_) {
        if (streaming_started_) {
            VxStopStreaming(camera_);
            streaming_started_ = false;
        }
        VxClose(camera_);
        camera_.reset();
    }
}

void VizionSdkCameraNode::ConfigureIspControls() {
    for (const auto& parameter : IspControlParameters()) {
        const int value = this->get_parameter(parameter.parameter_name).as_int();
        if (value < 0) {
            continue;
        }

        std::string error;
        if (!ApplyIspControl(parameter.property, value, error)) {
            throw std::runtime_error(error);
        }
    }

    parameter_callback_ = this->add_on_set_parameters_callback(
        [this](const std::vector<rclcpp::Parameter>& parameters) {
            return OnSetParameters(parameters);
        });
}

void VizionSdkCameraNode::ConfigureStatus() {
    if (!publish_status_) {
        return;
    }

    status_pub_ = this->create_publisher<diagnostic_msgs::msg::DiagnosticArray>(
        "camera/status", rclcpp::QoS(10));
    const auto period = ToNanoseconds(std::chrono::duration<double>(1.0 / status_rate_hz_));
    status_timer_ = this->create_wall_timer(period, [this]() { PublishStatus(); });
}

void VizionSdkCameraNode::ConfigureImu() {
    if (!publish_imu_) {
        return;
    }
    if (imu_mode_ == VX_IMU_MODE::DISABLE) {
        RCLCPP_WARN(this->get_logger(), "publish_imu is true but imu_mode is disable");
        return;
    }

    if (VxEnableIMUMode(camera_, imu_mode_) != 0) {
        throw std::runtime_error("VxEnableIMUMode failed");
    }

    imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>("imu/data", rclcpp::SensorDataQoS());
    const auto period = ToNanoseconds(std::chrono::duration<double>(1.0 / imu_rate_hz_));
    imu_timer_ = this->create_wall_timer(period, [this]() { PublishImu(); });
}

void VizionSdkCameraNode::ConfigureImage() {
    if (!publish_image_) {
        return;
    }

    std::vector<VxFormat> formats;
    if (VxGetFormatList(camera_, formats) != 0 || formats.empty()) {
        throw std::runtime_error("VxGetFormatList failed or returned no formats");
    }

    const auto selected = SelectImageFormat(formats, image_request_);
    if (!selected.has_value()) {
        throw std::runtime_error("No camera format matched " + FormatRequestDescription(image_request_));
    }
    image_format_ = *selected;

    if (VxSetFormat(camera_, image_format_) != 0) {
        throw std::runtime_error("VxSetFormat failed for " + FormatRequestDescription(image_request_));
    }
    if (VxStartStreaming(camera_) != 0) {
        throw std::runtime_error("VxStartStreaming failed");
    }
    streaming_started_ = true;

    image_buffer_.resize(
        CalculateImageBufferSize(image_format_.width, image_format_.height, image_format_.format));

    if (IsCompressedImageFormat(image_format_.format)) {
        compressed_image_pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>(
            "image_raw/compressed", rclcpp::SensorDataQoS());
    } else {
        image_pub_ =
            this->create_publisher<sensor_msgs::msg::Image>("image_raw", rclcpp::SensorDataQoS());
    }

    camera_info_pub_ =
        this->create_publisher<sensor_msgs::msg::CameraInfo>("camera_info", rclcpp::QoS(10));
    LoadCameraInfo();

    const double selected_rate = image_rate_hz_ > 0.0
                                     ? image_rate_hz_
                                     : static_cast<double>(std::max<uint16_t>(image_format_.framerate, 1));
    const auto period = ToNanoseconds(std::chrono::duration<double>(1.0 / selected_rate));
    image_timer_ = this->create_wall_timer(period, [this]() { PublishImage(); });

    RCLCPP_INFO(this->get_logger(), "Publishing image format %s %ux%u @ %u fps",
                FormatName(image_format_.format).c_str(), image_format_.width, image_format_.height,
                image_format_.framerate);
}

void VizionSdkCameraNode::LoadCameraInfo() {
    VxIntrinsics intrinsics{};
    if (VxGetIntrinsics(camera_, intrinsics) != 0) {
        RCLCPP_WARN(this->get_logger(), "VxGetIntrinsics failed; camera_info will not be published");
        camera_info_.reset();
        return;
    }

    const auto calibration = MakeIntrinsicsCalibration(intrinsics);
    sensor_msgs::msg::CameraInfo msg;
    msg.header.frame_id = image_frame_id_;
    msg.width = calibration.width;
    msg.height = calibration.height;
    msg.distortion_model = calibration.distortion_model;
    msg.d.assign(calibration.distortion_coefficients.begin(),
                 calibration.distortion_coefficients.end());
    std::copy(calibration.k.begin(), calibration.k.end(), msg.k.begin());
    std::copy(calibration.r.begin(), calibration.r.end(), msg.r.begin());
    std::copy(calibration.p.begin(), calibration.p.end(), msg.p.begin());
    camera_info_ = std::move(msg);
}

void VizionSdkCameraNode::PublishImu() {
    std::vector<float> acceleration;
    std::vector<float> gyroscope;

    if (VxGetIMUAccData(camera_, acceleration, self_test_mode_) != 0) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                             "VxGetIMUAccData failed");
        return;
    }
    if (VxGetIMUGyrData(camera_, gyroscope, self_test_mode_) != 0) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                             "VxGetIMUGyrData failed");
        return;
    }

    std::optional<VxImuIspu> ispu;
    if (publish_ispu_orientation_) {
        VxImuIspu ispu_data{};
        if (VxGetISPUData(camera_, ispu_data) == 0) {
            ispu = ispu_data;
        } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                                 "VxGetISPUData failed; publishing IMU without orientation");
        }
    }

    const auto sample = MakeImuSample(acceleration, gyroscope, acceleration_unit_, gyro_unit_, ispu,
                                      publish_ispu_orientation_);

    sensor_msgs::msg::Imu msg;
    msg.header.stamp = this->now();
    msg.header.frame_id = imu_frame_id_;
    msg.linear_acceleration.x = sample.linear_acceleration.x;
    msg.linear_acceleration.y = sample.linear_acceleration.y;
    msg.linear_acceleration.z = sample.linear_acceleration.z;
    msg.angular_velocity.x = sample.angular_velocity.x;
    msg.angular_velocity.y = sample.angular_velocity.y;
    msg.angular_velocity.z = sample.angular_velocity.z;

    if (sample.orientation.has_value()) {
        msg.orientation.x = sample.orientation->x;
        msg.orientation.y = sample.orientation->y;
        msg.orientation.z = sample.orientation->z;
        msg.orientation.w = sample.orientation->w;
    } else {
        msg.orientation_covariance[0] = -1.0;
    }

    imu_pub_->publish(msg);
}

void VizionSdkCameraNode::PublishImage() {
    int data_size = static_cast<int>(image_buffer_.size());
    const auto result = VxGetImage(camera_, image_buffer_.data(), &data_size,
                                   static_cast<uint16_t>(image_timeout_ms_));
    if (result != VX_CAPTURE_RESULT::VX_SUCCESS || data_size <= 0) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                             "VxGetImage failed with result %d", static_cast<int>(result));
        return;
    }
    const auto bytes_received = static_cast<size_t>(data_size);
    if (bytes_received > image_buffer_.size()) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                             "VxGetImage returned %zu bytes for a %zu byte buffer", bytes_received,
                             image_buffer_.size());
        return;
    }

    const auto stamp = this->now();

    if (IsCompressedImageFormat(image_format_.format)) {
        sensor_msgs::msg::CompressedImage msg;
        msg.header.stamp = stamp;
        msg.header.frame_id = image_frame_id_;
        msg.format = CompressedImageFormat(image_format_.format);
        msg.data.assign(image_buffer_.begin(), image_buffer_.begin() + bytes_received);
        compressed_image_pub_->publish(std::move(msg));
    } else {
        const size_t step = CalculateImageStep(image_format_.width, image_format_.format);
        if (step == 0U) {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                                 "Cannot publish raw image for format %s",
                                 FormatName(image_format_.format).c_str());
            return;
        }

        sensor_msgs::msg::Image msg;
        msg.header.stamp = stamp;
        msg.header.frame_id = image_frame_id_;
        msg.height = image_format_.height;
        msg.width = image_format_.width;
        msg.encoding = RosImageEncoding(image_format_.format);
        msg.is_bigendian = false;
        msg.step = static_cast<uint32_t>(step);
        msg.data.assign(image_buffer_.begin(), image_buffer_.begin() + bytes_received);
        image_pub_->publish(std::move(msg));
    }

    PublishCameraInfo(stamp);
}

void VizionSdkCameraNode::PublishCameraInfo(const rclcpp::Time& stamp) {
    if (!camera_info_.has_value() || !camera_info_pub_) {
        return;
    }

    auto msg = *camera_info_;
    msg.header.stamp = stamp;
    msg.header.frame_id = image_frame_id_;
    camera_info_pub_->publish(std::move(msg));
}

void VizionSdkCameraNode::PublishStatus() {
    if (!status_pub_) {
        return;
    }

    diagnostic_msgs::msg::DiagnosticStatus status;
    status.name = "vizionsdk_camera_controls";
    status.hardware_id = "device_index_" + std::to_string(device_index_);
    status.level = diagnostic_msgs::msg::DiagnosticStatus::OK;
    status.message = "OK";

    auto add_value = [&status](const std::string& key, const std::string& value) {
        diagnostic_msgs::msg::KeyValue item;
        item.key = key;
        item.value = value;
        status.values.push_back(std::move(item));
    };

    uint32_t exposure{};
    if (VxGetCurrentExposure(camera_, exposure) == 0) {
        add_value("current_exposure", std::to_string(exposure));
    } else {
        status.level = diagnostic_msgs::msg::DiagnosticStatus::WARN;
        add_value("current_exposure", "unavailable");
    }

    uint8_t gain{};
    if (VxGetCurrentGain(camera_, gain) == 0) {
        add_value("current_gain", std::to_string(static_cast<unsigned int>(gain)));
    } else {
        status.level = diagnostic_msgs::msg::DiagnosticStatus::WARN;
        add_value("current_gain", "unavailable");
    }

    for (const auto property : {VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_MODE,
                               VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_TIME,
                               VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_GAIN}) {
        const std::string name = IspPropertyName(property);
        int value{};
        int flag{};
        if (VxGetISPImageProcessing(camera_, property, value, flag) == 0) {
            add_value("isp." + name + ".value", std::to_string(value));
            add_value("isp." + name + ".auto_flag", std::to_string(flag));
        }

        int min{};
        int max{};
        int step{};
        int def{};
        if (VxGetISPImageProcessingRange(camera_, property, min, max, step, def) == 0) {
            add_value("isp." + name + ".min", std::to_string(min));
            add_value("isp." + name + ".max", std::to_string(max));
            add_value("isp." + name + ".step", std::to_string(step));
            add_value("isp." + name + ".default", std::to_string(def));
        }
    }

    diagnostic_msgs::msg::DiagnosticArray msg;
    msg.header.stamp = this->now();
    msg.status.push_back(std::move(status));
    status_pub_->publish(std::move(msg));
}

rcl_interfaces::msg::SetParametersResult VizionSdkCameraNode::OnSetParameters(
    const std::vector<rclcpp::Parameter>& parameters) {
    rcl_interfaces::msg::SetParametersResult result;
    result.successful = true;

    for (const auto& parameter : parameters) {
        const auto control = IspControlParameterFor(parameter.get_name());
        if (!control.has_value()) {
            continue;
        }
        if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_INTEGER) {
            result.successful = false;
            result.reason = parameter.get_name() + " must be an integer";
            return result;
        }

        const int value = parameter.as_int();
        if (value < 0) {
            continue;
        }

        std::string error;
        if (!ApplyIspControl(control->property, value, error)) {
            result.successful = false;
            result.reason = error;
            return result;
        }
    }

    return result;
}

bool VizionSdkCameraNode::ApplyIspControl(VX_ISP_IMAGE_PROPERTIES property,
                                          int value,
                                          std::string& error) {
    int min{};
    int max{};
    int step{};
    int def{};
    const std::string name = IspPropertyName(property);
    if (VxGetISPImageProcessingRange(camera_, property, min, max, step, def) != 0) {
        error = "VxGetISPImageProcessingRange failed for isp." + name;
        return false;
    }
    if (value < min || value > max) {
        error = "isp." + name + " must be between " + std::to_string(min) + " and " +
                std::to_string(max);
        return false;
    }
    if (step > 0 && ((value - min) % step) != 0) {
        error = "isp." + name + " must follow step " + std::to_string(step) +
                " from min " + std::to_string(min);
        return false;
    }
    if (VxSetISPImageProcessing(camera_, property, value) != 0) {
        error = "VxSetISPImageProcessing failed for isp." + name;
        return false;
    }

    RCLCPP_INFO(this->get_logger(), "Set isp.%s=%d", name.c_str(), value);
    return true;
}

VX_IMU_MODE VizionSdkCameraNode::ParseImuModeParameter(const std::string& value) const {
    const std::string normalized = NormalizeToken(value);
    if (normalized == "disable" || normalized == "disabled" || normalized == "off") {
        return VX_IMU_MODE::DISABLE;
    }
    if (normalized == "ispu") {
        return VX_IMU_MODE::ISPU;
    }
    if (normalized == "selftest") {
        return VX_IMU_MODE::SELF_TEST;
    }
    throw std::invalid_argument("imu_mode must be disable, ispu, or self_test");
}

VX_IMU_SELF_TEST_MODE VizionSdkCameraNode::ParseSelfTestModeParameter(const std::string& value) const {
    const std::string normalized = NormalizeToken(value);
    if (normalized == "normal") {
        return VX_IMU_SELF_TEST_MODE::NORMAL;
    }
    if (normalized == "positive") {
        return VX_IMU_SELF_TEST_MODE::POSITIVE;
    }
    if (normalized == "negative") {
        return VX_IMU_SELF_TEST_MODE::NEGATIVE;
    }
    if (normalized == "notallowed") {
        return VX_IMU_SELF_TEST_MODE::NOT_ALLOWED;
    }
    throw std::invalid_argument("imu_self_test_mode must be normal, positive, negative, or "
                                "not_allowed");
}

uint16_t VizionSdkCameraNode::ReadUint16Parameter(const std::string& name, int default_value) {
    const int value = this->declare_parameter<int>(name, default_value);
    if (value < 0 || value > 65535) {
        throw std::invalid_argument(name + " must be between 0 and 65535");
    }
    return static_cast<uint16_t>(value);
}

}  // namespace vizionsdk_ros2
