#pragma once

#include "VxPublicTypes.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace vizionsdk_ros2 {

constexpr double kStandardGravity = 9.80665;
constexpr double kPi = 3.14159265358979323846;

enum class AccelerationUnit {
    MetersPerSecondSquared,
    G,
    MilliG,
    Raw,
};

enum class GyroUnit {
    RadiansPerSecond,
    DegreesPerSecond,
    Raw,
};

struct Vector3 {
    double x{};
    double y{};
    double z{};
};

struct Quaternion {
    double x{};
    double y{};
    double z{};
    double w{1.0};
};

struct ImuSample {
    Vector3 linear_acceleration;
    Vector3 angular_velocity;
    std::optional<Quaternion> orientation;
};

struct IntrinsicsCalibration {
    uint32_t width{};
    uint32_t height{};
    std::string distortion_model;
    std::array<double, 5> distortion_coefficients{};
    std::array<double, 9> k{};
    std::array<double, 9> r{};
    std::array<double, 12> p{};
};

struct ImageFormatRequest {
    VX_IMAGE_FORMAT format{VX_IMAGE_FORMAT::NONE};
    uint16_t width{};
    uint16_t height{};
    uint16_t framerate{};
};

inline std::string NormalizeToken(std::string_view value) {
    std::string normalized;
    normalized.reserve(value.size());
    for (const unsigned char ch : value) {
        if (std::isspace(ch) || ch == '-' || ch == '_') {
            continue;
        }
        normalized.push_back(static_cast<char>(std::tolower(ch)));
    }
    return normalized;
}

inline std::string FormatName(VX_IMAGE_FORMAT format) {
    switch (format) {
        case VX_IMAGE_FORMAT::YUY2:
            return "YUY2";
        case VX_IMAGE_FORMAT::UYVY:
            return "UYVY";
        case VX_IMAGE_FORMAT::NV12:
            return "NV12";
        case VX_IMAGE_FORMAT::MJPG:
            return "MJPG";
        case VX_IMAGE_FORMAT::BGRA:
            return "BGRA";
        case VX_IMAGE_FORMAT::BGRX:
            return "BGRX";
        case VX_IMAGE_FORMAT::BGR:
            return "BGR";
        case VX_IMAGE_FORMAT::RGB16:
            return "RGB16";
        case VX_IMAGE_FORMAT::RGB:
            return "RGB";
        case VX_IMAGE_FORMAT::NONE:
        default:
            return "UNKNOWN";
    }
}

inline std::optional<VX_IMAGE_FORMAT> ParseImageFormat(std::string_view value) {
    const std::string normalized = NormalizeToken(value);
    if (normalized.empty() || normalized == "any" || normalized == "auto") {
        return VX_IMAGE_FORMAT::NONE;
    }
    if (normalized == "yuy2" || normalized == "yuyv") {
        return VX_IMAGE_FORMAT::YUY2;
    }
    if (normalized == "uyvy") {
        return VX_IMAGE_FORMAT::UYVY;
    }
    if (normalized == "nv12") {
        return VX_IMAGE_FORMAT::NV12;
    }
    if (normalized == "mjpg" || normalized == "mjpeg" || normalized == "jpeg") {
        return VX_IMAGE_FORMAT::MJPG;
    }
    if (normalized == "bgra") {
        return VX_IMAGE_FORMAT::BGRA;
    }
    if (normalized == "bgrx") {
        return VX_IMAGE_FORMAT::BGRX;
    }
    if (normalized == "bgr") {
        return VX_IMAGE_FORMAT::BGR;
    }
    if (normalized == "rgb16") {
        return VX_IMAGE_FORMAT::RGB16;
    }
    if (normalized == "rgb") {
        return VX_IMAGE_FORMAT::RGB;
    }
    return std::nullopt;
}

inline std::optional<AccelerationUnit> ParseAccelerationUnit(std::string_view value) {
    const std::string normalized = NormalizeToken(value);
    if (normalized == "m/s^2" || normalized == "m/s2" || normalized == "mps2" ||
        normalized == "meterspersecondsquared") {
        return AccelerationUnit::MetersPerSecondSquared;
    }
    if (normalized == "g") {
        return AccelerationUnit::G;
    }
    if (normalized == "mg" || normalized == "millig" || normalized == "milligravity") {
        return AccelerationUnit::MilliG;
    }
    if (normalized == "raw") {
        return AccelerationUnit::Raw;
    }
    return std::nullopt;
}

inline std::optional<GyroUnit> ParseGyroUnit(std::string_view value) {
    const std::string normalized = NormalizeToken(value);
    if (normalized == "rad/s" || normalized == "rads" || normalized == "radps" ||
        normalized == "radianspersecond") {
        return GyroUnit::RadiansPerSecond;
    }
    if (normalized == "deg/s" || normalized == "degs" || normalized == "degps" ||
        normalized == "degreespersecond") {
        return GyroUnit::DegreesPerSecond;
    }
    if (normalized == "raw") {
        return GyroUnit::Raw;
    }
    return std::nullopt;
}

inline bool IsCompressedImageFormat(VX_IMAGE_FORMAT format) {
    return format == VX_IMAGE_FORMAT::MJPG;
}

inline std::string CompressedImageFormat(VX_IMAGE_FORMAT format) {
    return format == VX_IMAGE_FORMAT::MJPG ? "jpeg" : "";
}

inline std::string RosImageEncoding(VX_IMAGE_FORMAT format) {
    switch (format) {
        case VX_IMAGE_FORMAT::YUY2:
            return "yuv422_yuy2";
        case VX_IMAGE_FORMAT::UYVY:
            return "uyvy";
        case VX_IMAGE_FORMAT::NV12:
            return "nv12";
        case VX_IMAGE_FORMAT::BGRA:
            return "bgra8";
        case VX_IMAGE_FORMAT::BGRX:
            return "bgrx8";
        case VX_IMAGE_FORMAT::BGR:
            return "bgr8";
        case VX_IMAGE_FORMAT::RGB16:
            return "rgb16";
        case VX_IMAGE_FORMAT::RGB:
            return "rgb8";
        case VX_IMAGE_FORMAT::MJPG:
            return "jpeg";
        case VX_IMAGE_FORMAT::NONE:
        default:
            return "";
    }
}

inline size_t CalculateImageBufferSize(size_t width, size_t height, VX_IMAGE_FORMAT format) {
    switch (format) {
        case VX_IMAGE_FORMAT::YUY2:
        case VX_IMAGE_FORMAT::UYVY:
            return width * height * 2U;
        case VX_IMAGE_FORMAT::NV12:
            return width * height * 3U / 2U;
        case VX_IMAGE_FORMAT::MJPG:
        case VX_IMAGE_FORMAT::BGRA:
        case VX_IMAGE_FORMAT::BGRX:
            return width * height * 4U;
        case VX_IMAGE_FORMAT::BGR:
        case VX_IMAGE_FORMAT::RGB:
            return width * height * 3U;
        case VX_IMAGE_FORMAT::RGB16:
            return width * height * 6U;
        case VX_IMAGE_FORMAT::NONE:
        default:
            return width * height * 3U;
    }
}

inline size_t CalculateImageStep(size_t width, VX_IMAGE_FORMAT format) {
    switch (format) {
        case VX_IMAGE_FORMAT::YUY2:
        case VX_IMAGE_FORMAT::UYVY:
            return width * 2U;
        case VX_IMAGE_FORMAT::NV12:
            return width;
        case VX_IMAGE_FORMAT::BGRA:
        case VX_IMAGE_FORMAT::BGRX:
            return width * 4U;
        case VX_IMAGE_FORMAT::BGR:
        case VX_IMAGE_FORMAT::RGB:
            return width * 3U;
        case VX_IMAGE_FORMAT::RGB16:
            return width * 6U;
        case VX_IMAGE_FORMAT::MJPG:
        case VX_IMAGE_FORMAT::NONE:
        default:
            return 0U;
    }
}

inline bool MatchesFormatRequest(const VxFormat& candidate, const ImageFormatRequest& request) {
    if (candidate.format == VX_IMAGE_FORMAT::NONE) {
        return false;
    }
    if (request.format != VX_IMAGE_FORMAT::NONE && candidate.format != request.format) {
        return false;
    }
    if (request.width != 0 && candidate.width != request.width) {
        return false;
    }
    if (request.height != 0 && candidate.height != request.height) {
        return false;
    }
    if (request.framerate != 0 && candidate.framerate != request.framerate) {
        return false;
    }
    return true;
}

inline std::optional<VxFormat> SelectImageFormat(const std::vector<VxFormat>& formats,
                                                 const ImageFormatRequest& request) {
    for (const auto& format : formats) {
        if (MatchesFormatRequest(format, request)) {
            return format;
        }
    }
    return std::nullopt;
}

inline double AccelerationScale(AccelerationUnit unit) {
    switch (unit) {
        case AccelerationUnit::G:
            return kStandardGravity;
        case AccelerationUnit::MilliG:
            return kStandardGravity / 1000.0;
        case AccelerationUnit::MetersPerSecondSquared:
        case AccelerationUnit::Raw:
        default:
            return 1.0;
    }
}

inline double GyroScale(GyroUnit unit) {
    switch (unit) {
        case GyroUnit::DegreesPerSecond:
            return kPi / 180.0;
        case GyroUnit::RadiansPerSecond:
        case GyroUnit::Raw:
        default:
            return 1.0;
    }
}

inline Vector3 VectorFromFloats(const std::vector<float>& values, double scale) {
    return {
        values.size() > 0 ? static_cast<double>(values[0]) * scale : 0.0,
        values.size() > 1 ? static_cast<double>(values[1]) * scale : 0.0,
        values.size() > 2 ? static_cast<double>(values[2]) * scale : 0.0,
    };
}

inline ImuSample MakeImuSample(const std::vector<float>& acceleration,
                               const std::vector<float>& gyroscope,
                               AccelerationUnit acceleration_unit,
                               GyroUnit gyro_unit,
                               const std::optional<VxImuIspu>& ispu = std::nullopt,
                               bool use_ispu_orientation = false) {
    ImuSample sample;
    sample.linear_acceleration = VectorFromFloats(acceleration, AccelerationScale(acceleration_unit));
    sample.angular_velocity = VectorFromFloats(gyroscope, GyroScale(gyro_unit));

    if (use_ispu_orientation && ispu.has_value()) {
        sample.orientation = Quaternion{
            static_cast<double>(ispu->x),
            static_cast<double>(ispu->y),
            static_cast<double>(ispu->z),
            static_cast<double>(ispu->w),
        };
    }

    return sample;
}

inline IntrinsicsCalibration MakeIntrinsicsCalibration(const VxIntrinsics& intrinsics) {
    IntrinsicsCalibration calibration;
    calibration.width = static_cast<uint32_t>(intrinsics.w);
    calibration.height = static_cast<uint32_t>(intrinsics.h);
    calibration.distortion_model =
        intrinsics.model == VX_DISTORTION_MODEL::VX_DIST_NONE ? "none" : "plumb_bob";

    // VizionSDK stores RADTAN coefficients as k1, k2, k3, r1, r2.
    // ROS plumb_bob expects k1, k2, t1, t2, k3.
    calibration.distortion_coefficients = {
        static_cast<double>(intrinsics.coeffs[0]),
        static_cast<double>(intrinsics.coeffs[1]),
        static_cast<double>(intrinsics.coeffs[3]),
        static_cast<double>(intrinsics.coeffs[4]),
        static_cast<double>(intrinsics.coeffs[2]),
    };

    calibration.k = {
        static_cast<double>(intrinsics.fx), 0.0, static_cast<double>(intrinsics.cx),
        0.0, static_cast<double>(intrinsics.fy), static_cast<double>(intrinsics.cy),
        0.0, 0.0, 1.0,
    };
    calibration.r = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
    };
    calibration.p = {
        static_cast<double>(intrinsics.fx), 0.0, static_cast<double>(intrinsics.cx), 0.0,
        0.0, static_cast<double>(intrinsics.fy), static_cast<double>(intrinsics.cy), 0.0,
        0.0, 0.0, 1.0, 0.0,
    };

    return calibration;
}

}  // namespace vizionsdk_ros2
