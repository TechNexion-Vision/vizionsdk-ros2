#include "vizionsdk_ros2/yuv422_converter.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void ExpectTrue(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void ExpectNearByte(uint8_t actual, uint8_t expected, uint8_t tolerance, const std::string& label) {
    const int delta = static_cast<int>(actual) - static_cast<int>(expected);
    if (delta < -static_cast<int>(tolerance) || delta > static_cast<int>(tolerance)) {
        throw std::runtime_error(label + " expected near " + std::to_string(expected) + ", got " +
                                 std::to_string(actual));
    }
}

void ExpectPixelNear(const std::vector<uint8_t>& bgr,
                     size_t pixel_index,
                     uint8_t b,
                     uint8_t g,
                     uint8_t r,
                     uint8_t tolerance = 1U) {
    const size_t offset = pixel_index * 3U;
    ExpectNearByte(bgr.at(offset), b, tolerance, "pixel " + std::to_string(pixel_index) + " b");
    ExpectNearByte(bgr.at(offset + 1U), g, tolerance,
                   "pixel " + std::to_string(pixel_index) + " g");
    ExpectNearByte(bgr.at(offset + 2U), r, tolerance,
                   "pixel " + std::to_string(pixel_index) + " r");
}

}  // namespace

int main() {
    using namespace vizionsdk_ros2;

    const std::vector<uint8_t> uyvy_black_white = {
        128, 16, 128, 235,
    };
    const auto uyvy_bgr = ConvertYuv422ToBgr(uyvy_black_white, 2, 1, "uyvy");
    ExpectTrue(uyvy_bgr.size() == 6U, "UYVY output size must be 2 pixels of bgr8");
    ExpectPixelNear(uyvy_bgr, 0, 0, 0, 0);
    ExpectPixelNear(uyvy_bgr, 1, 255, 255, 255);

    const std::vector<uint8_t> yuy2_red_blue = {
        82, 90, 82, 240,
        41, 240, 41, 110,
    };
    const auto yuy2_bgr = ConvertYuv422ToBgr(yuy2_red_blue, 4, 1, "yuv422_yuy2");
    ExpectTrue(yuy2_bgr.size() == 12U, "YUY2 output size must be 4 pixels of bgr8");
    ExpectPixelNear(yuy2_bgr, 0, 0, 0, 255);
    ExpectPixelNear(yuy2_bgr, 1, 0, 0, 255);
    ExpectPixelNear(yuy2_bgr, 2, 255, 0, 0);
    ExpectPixelNear(yuy2_bgr, 3, 255, 0, 0);

    bool threw = false;
    try {
        (void)ConvertYuv422ToBgr(uyvy_black_white, 2, 1, "rgb8");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    ExpectTrue(threw, "unsupported encoding must throw");

    std::cout << "yuv422 converter tests passed\n";
    return 0;
}
