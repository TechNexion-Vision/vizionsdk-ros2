#include "vizionsdk_ros2/yuv422_converter.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void ExpectPixel(const std::vector<uint8_t>& bgr, size_t pixel_index, uint8_t b, uint8_t g,
                 uint8_t r) {
    const size_t offset = pixel_index * 3U;
    assert(bgr.at(offset) == b);
    assert(bgr.at(offset + 1U) == g);
    assert(bgr.at(offset + 2U) == r);
}

}  // namespace

int main() {
    using namespace vizionsdk_ros2;

    const std::vector<uint8_t> uyvy_black_white = {
        128, 16, 128, 235,
    };
    const auto uyvy_bgr = ConvertYuv422ToBgr(uyvy_black_white, 2, 1, "uyvy");
    assert(uyvy_bgr.size() == 6U);
    ExpectPixel(uyvy_bgr, 0, 0, 0, 0);
    ExpectPixel(uyvy_bgr, 1, 255, 255, 255);

    const std::vector<uint8_t> yuy2_red_blue = {
        82, 90, 41, 240,
    };
    const auto yuy2_bgr = ConvertYuv422ToBgr(yuy2_red_blue, 2, 1, "yuv422_yuy2");
    ExpectPixel(yuy2_bgr, 0, 0, 0, 255);
    ExpectPixel(yuy2_bgr, 1, 255, 0, 0);

    bool threw = false;
    try {
        (void)ConvertYuv422ToBgr(uyvy_black_white, 2, 1, "rgb8");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "yuv422 converter tests passed\n";
    return 0;
}
