#include "vizionsdk_ros2/list_devices.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "test_assertions.hpp"

int main() {
    using namespace vizionsdk_ros2;

    assert(InterfaceTypeName(VX_CAMERA_INTERFACE_TYPE::INTERFACE_USB) == "USB");
    assert(InterfaceTypeName(VX_CAMERA_INTERFACE_TYPE::INTERFACE_MIPI_CSI2) == "MIPI_CSI2");
    assert(InterfaceTypeName(VX_CAMERA_INTERFACE_TYPE::INTERFACE_ETHERNET) == "ETHERNET");

    const std::vector<DeviceInfo> devices = {
        {
            0,
            "VCM-AR0234-C",
            "/dev/video2",
            VX_CAMERA_INTERFACE_TYPE::INTERFACE_USB,
            "1234",
            "SN0001",
            true,
        },
        {
            1,
            "Unknown Vizion camera",
            "",
            VX_CAMERA_INTERFACE_TYPE::INTERFACE_MIPI_CSI2,
            "",
            "",
            false,
        },
    };

    const std::string output = FormatDeviceList(devices);
    assert(output.find("[0] VCM-AR0234-C") != std::string::npos);
    assert(output.find("path: /dev/video2") != std::string::npos);
    assert(output.find("interface: USB") != std::string::npos);
    assert(output.find("hardware_id: 1234") != std::string::npos);
    assert(output.find("serial: SN0001") != std::string::npos);
    assert(output.find("[1] Unknown Vizion camera") != std::string::npos);
    assert(output.find("interface: MIPI_CSI2") != std::string::npos);
    assert(output.find("warning: failed to open device details") != std::string::npos);

    assert(FormatDeviceList({}) == "No Vizion camera devices found.\n");

    std::cout << "list device tests passed\n";
    return 0;
}
