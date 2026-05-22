#pragma once

#include <algorithm>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace vizionsdk_ros2 {

namespace detail {

inline uint8_t ClampToByte(int value) {
    return static_cast<uint8_t>(std::clamp(value, 0, 255));
}

inline void AppendYuvPixelAsBgr(std::vector<uint8_t>& output, uint8_t y, uint8_t u, uint8_t v) {
    const int c = static_cast<int>(y) - 16;
    const int d = static_cast<int>(u) - 128;
    const int e = static_cast<int>(v) - 128;

    const int r = (298 * c + 409 * e + 128) >> 8;
    const int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
    const int b = (298 * c + 516 * d + 128) >> 8;

    output.push_back(ClampToByte(b));
    output.push_back(ClampToByte(g));
    output.push_back(ClampToByte(r));
}

}  // namespace detail

inline bool IsSupportedYuv422Encoding(std::string_view encoding) {
    return encoding == "uyvy" || encoding == "yuv422_yuy2";
}

inline std::vector<uint8_t> ConvertYuv422ToBgr(std::span<const uint8_t> input, size_t width,
                                               size_t height, std::string_view encoding) {
    if (!IsSupportedYuv422Encoding(encoding)) {
        throw std::invalid_argument("encoding must be uyvy or yuv422_yuy2");
    }
    if ((width % 2U) != 0U) {
        throw std::invalid_argument("YUV422 image width must be even");
    }

    const size_t expected_size = width * height * 2U;
    if (input.size() != expected_size) {
        throw std::invalid_argument("YUV422 input size does not match width and height");
    }

    std::vector<uint8_t> output;
    output.reserve(width * height * 3U);

    for (size_t offset = 0; offset < input.size(); offset += 4U) {
        if (encoding == "uyvy") {
            const uint8_t u = input[offset];
            const uint8_t y0 = input[offset + 1U];
            const uint8_t v = input[offset + 2U];
            const uint8_t y1 = input[offset + 3U];
            detail::AppendYuvPixelAsBgr(output, y0, u, v);
            detail::AppendYuvPixelAsBgr(output, y1, u, v);
        } else {
            const uint8_t y0 = input[offset];
            const uint8_t u = input[offset + 1U];
            const uint8_t y1 = input[offset + 2U];
            const uint8_t v = input[offset + 3U];
            detail::AppendYuvPixelAsBgr(output, y0, u, v);
            detail::AppendYuvPixelAsBgr(output, y1, u, v);
        }
    }

    return output;
}

}  // namespace vizionsdk_ros2
