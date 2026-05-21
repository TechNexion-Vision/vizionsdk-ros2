#include "vizionsdk_ros2/conversions.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

void ExpectNear(double actual, double expected, double tolerance = 1e-6) {
    assert(std::fabs(actual - expected) <= tolerance);
}

}  // namespace

int main() {
    using namespace vizionsdk_ros2;

    assert(FormatName(VX_IMAGE_FORMAT::YUY2) == "YUY2");
    assert(FormatName(VX_IMAGE_FORMAT::MJPG) == "MJPG");
    assert(RosImageEncoding(VX_IMAGE_FORMAT::BGR) == "bgr8");
    assert(RosImageEncoding(VX_IMAGE_FORMAT::RGB) == "rgb8");
    assert(RosImageEncoding(VX_IMAGE_FORMAT::YUY2) == "yuv422_yuy2");
    assert(IsCompressedImageFormat(VX_IMAGE_FORMAT::MJPG));
    assert(!IsCompressedImageFormat(VX_IMAGE_FORMAT::BGR));

    assert(CalculateImageBufferSize(640, 480, VX_IMAGE_FORMAT::YUY2) == 640U * 480U * 2U);
    assert(CalculateImageBufferSize(640, 480, VX_IMAGE_FORMAT::NV12) == 640U * 480U * 3U / 2U);
    assert(CalculateImageBufferSize(640, 480, VX_IMAGE_FORMAT::MJPG) == 640U * 480U * 4U);
    assert(CalculateImageStep(640, VX_IMAGE_FORMAT::RGB16) == 640U * 6U);

    const std::vector<VxFormat> formats = {
        {0, 1920, 1080, 30, VX_IMAGE_FORMAT::NONE},
        {1, 640, 480, 30, VX_IMAGE_FORMAT::BGR},
        {2, 1280, 720, 60, VX_IMAGE_FORMAT::MJPG},
        {3, 1280, 720, 30, VX_IMAGE_FORMAT::MJPG},
    };
    const auto selected = SelectImageFormat(
        formats, ImageFormatRequest{VX_IMAGE_FORMAT::MJPG, 1280, 720, 0});
    assert(selected.has_value());
    assert(selected->mediatypeIdx == 2);

    const auto fallback = SelectImageFormat(formats, ImageFormatRequest{});
    assert(fallback.has_value());
    assert(fallback->format == VX_IMAGE_FORMAT::BGR);

    assert(ParseImageFormat("mjpg").value() == VX_IMAGE_FORMAT::MJPG);
    assert(ParseImageFormat("YUYV").value() == VX_IMAGE_FORMAT::YUY2);
    assert(ParseAccelerationUnit("m/s^2").value() ==
           AccelerationUnit::MetersPerSecondSquared);
    assert(ParseAccelerationUnit("mg").value() == AccelerationUnit::MilliG);
    assert(ParseGyroUnit("rad/s").value() == GyroUnit::RadiansPerSecond);

    VxImuIspu ispu{};
    ispu.x = 0.1F;
    ispu.y = 0.2F;
    ispu.z = 0.3F;
    ispu.w = 0.9F;

    const auto imu = MakeImuSample({1.0F, -2.0F, 0.5F},
                                  {90.0F, -180.0F, 45.0F},
                                  AccelerationUnit::G,
                                  GyroUnit::DegreesPerSecond,
                                  ispu,
                                  true);
    ExpectNear(imu.linear_acceleration.x, 9.80665);
    ExpectNear(imu.linear_acceleration.y, -19.6133);
    ExpectNear(imu.angular_velocity.x, 1.5707963267948966);
    ExpectNear(imu.angular_velocity.y, -3.141592653589793);
    assert(imu.orientation.has_value());
    ExpectNear(imu.orientation->w, 0.9);

    const auto sdk_imu = MakeImuSample({1000.0F, 0.0F, -1000.0F},
                                      {90.0F, 0.0F, -90.0F},
                                      AccelerationUnit::MilliG,
                                      GyroUnit::DegreesPerSecond);
    ExpectNear(sdk_imu.linear_acceleration.x, 9.80665);
    ExpectNear(sdk_imu.linear_acceleration.z, -9.80665);
    ExpectNear(sdk_imu.angular_velocity.x, 1.5707963267948966);

    VxIntrinsics intrinsics{};
    intrinsics.w = 1280;
    intrinsics.h = 720;
    intrinsics.fx = 600.0F;
    intrinsics.fy = 610.0F;
    intrinsics.cx = 640.0F;
    intrinsics.cy = 360.0F;
    intrinsics.model = VX_DISTORTION_MODEL::VX_DIST_MODEL_RADTAN;
    intrinsics.coeffs[0] = 0.1F;
    intrinsics.coeffs[1] = 0.2F;
    intrinsics.coeffs[2] = 0.3F;
    intrinsics.coeffs[3] = 0.4F;
    intrinsics.coeffs[4] = 0.5F;

    const auto calibration = MakeIntrinsicsCalibration(intrinsics);
    assert(calibration.width == 1280U);
    assert(calibration.height == 720U);
    assert(calibration.distortion_model == "plumb_bob");
    ExpectNear(calibration.k[0], 600.0);
    ExpectNear(calibration.k[2], 640.0);
    ExpectNear(calibration.k[4], 610.0);
    ExpectNear(calibration.p[0], 600.0);
    ExpectNear(calibration.p[5], 610.0);
    ExpectNear(calibration.distortion_coefficients[0], 0.1);
    ExpectNear(calibration.distortion_coefficients[1], 0.2);
    ExpectNear(calibration.distortion_coefficients[2], 0.4);
    ExpectNear(calibration.distortion_coefficients[3], 0.5);
    ExpectNear(calibration.distortion_coefficients[4], 0.3);

    std::cout << "conversion tests passed\n";
    return 0;
}
