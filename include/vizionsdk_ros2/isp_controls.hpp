#pragma once

#include "VxPublicTypes.hpp"
#include "vizionsdk_ros2/conversions.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace vizionsdk_ros2 {

struct IspControlParameter {
    std::string parameter_name;
    VX_ISP_IMAGE_PROPERTIES property;
};

inline const std::vector<IspControlParameter>& IspControlParameters() {
    static const std::vector<IspControlParameter> parameters = {
        {"isp.brightness", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_BRIGHTNESS},
        {"isp.contrast", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_CONTRAST},
        {"isp.saturation", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_SATURATION},
        {"isp.whitebalance_mode", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_WHITEBALANCE_MODE},
        {"isp.whitebalance_temperature",
         VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_WHITEBALANCE_TEMPERATURE},
        {"isp.exposure_mode", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_MODE},
        {"isp.exposure_time", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_TIME},
        {"isp.exposure_min_time", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_MIN_TIME},
        {"isp.exposure_max_time", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_MAX_TIME},
        {"isp.exposure_gain", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_GAIN},
        {"isp.gamma", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_GAMMA},
        {"isp.sharpness", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_SHARPNESS},
        {"isp.backlight_compensation",
         VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_BACKLIGHT_COMPENSATION},
        {"isp.special_effect_mode", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_SPECIAL_EFFECT_MODE},
        {"isp.denoise", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_DENOISE},
        {"isp.flip_mode", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_FLIP_MODE},
        {"isp.pan", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_PAN},
        {"isp.tilt", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_TILT},
        {"isp.zoom", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_ZOOM},
        {"isp.flick_mode", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_FLICK_MODE},
        {"isp.jpeg_quality", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_JPEG_QUALITY},
        {"isp.trigger_mode", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_TRIGGER_MODE},
        {"isp.ehdr_mode", VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EHDR_MODE},
        {"isp.ehdr_exposure_min_number",
         VX_ISP_IMAGE_PROPERTIES::ISP_EHDR_EXPOSURE_MIN_NUMBER},
        {"isp.ehdr_exposure_max_number",
         VX_ISP_IMAGE_PROPERTIES::ISP_EHDR_EXPOSURE_MAX_NUMBER},
        {"isp.ehdr_ratio_min", VX_ISP_IMAGE_PROPERTIES::ISP_EHDR_RATIO_MIN},
        {"isp.ehdr_ratio_max", VX_ISP_IMAGE_PROPERTIES::ISP_EHDR_RATIO_MAX},
        {"isp.ehdr_atm_weight", VX_ISP_IMAGE_PROPERTIES::ISP_EHDR_ATM_WEIGHT},
        {"isp.ehdr_atm_max", VX_ISP_IMAGE_PROPERTIES::ISP_EHDR_ATM_MAX},
    };
    return parameters;
}

inline std::optional<IspControlParameter> IspControlParameterFor(std::string_view parameter_name) {
    for (const auto& parameter : IspControlParameters()) {
        if (parameter.parameter_name == parameter_name) {
            return parameter;
        }
    }
    return std::nullopt;
}

inline std::string IspPropertyName(VX_ISP_IMAGE_PROPERTIES property) {
    for (const auto& parameter : IspControlParameters()) {
        if (parameter.property == property) {
            return parameter.parameter_name.substr(4);
        }
    }
    return "unknown";
}

inline std::optional<VX_ISP_IMAGE_PROPERTIES> ParseIspProperty(std::string_view value) {
    const std::string normalized = NormalizeToken(value);
    for (const auto& parameter : IspControlParameters()) {
        const std::string short_name = parameter.parameter_name.substr(4);
        if (NormalizeToken(short_name) == normalized ||
            NormalizeToken(parameter.parameter_name) == normalized ||
            NormalizeToken("ISP_IMAGE_" + short_name) == normalized ||
            NormalizeToken("ISP_" + short_name) == normalized) {
            return parameter.property;
        }
    }
    return std::nullopt;
}

}  // namespace vizionsdk_ros2
