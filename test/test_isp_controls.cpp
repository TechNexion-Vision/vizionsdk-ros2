#include "vizionsdk_ros2/isp_controls.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace vizionsdk_ros2;

    assert(IspPropertyName(VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_TIME) == "exposure_time");
    assert(IspPropertyName(VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_GAIN) == "exposure_gain");
    assert(ParseIspProperty("exposure_time").value() ==
           VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_TIME);
    assert(ParseIspProperty("ISP_IMAGE_EXPOSURE_GAIN").value() ==
           VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_GAIN);
    assert(!ParseIspProperty("not_a_property").has_value());

    const auto parameters = IspControlParameters();
    assert(!parameters.empty());
    assert(parameters.front().parameter_name.rfind("isp.", 0) == 0);

    const auto exposure = IspControlParameterFor("isp.exposure_time");
    assert(exposure.has_value());
    assert(exposure->property == VX_ISP_IMAGE_PROPERTIES::ISP_IMAGE_EXPOSURE_TIME);

    const auto missing = IspControlParameterFor("image_format");
    assert(!missing.has_value());

    std::cout << "isp control tests passed\n";
    return 0;
}
