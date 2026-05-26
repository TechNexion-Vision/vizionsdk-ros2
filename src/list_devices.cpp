#include "vizionsdk_ros2/list_devices.hpp"

#include "VizionSDK.h"

#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

std::vector<vizionsdk_ros2::DeviceInfo> DiscoverDevices(bool include_formats) {
    std::vector<std::string> discovered;
    const int count = VxDiscoverCameraDevices(discovered);
    if (count <= 0 || discovered.empty()) {
        return {};
    }

    std::vector<vizionsdk_ros2::DeviceInfo> devices;
    devices.reserve(discovered.size());

    for (size_t i = 0; i < discovered.size(); ++i) {
        vizionsdk_ros2::DeviceInfo info;
        info.index = static_cast<int>(i);
        info.name = discovered[i];
        info.formats_requested = include_formats;

        auto camera = VxInitialCameraDevice(info.index);
        if (!camera) {
            devices.push_back(std::move(info));
            continue;
        }

        if (VxOpen(camera) != 0) {
            devices.push_back(std::move(info));
            continue;
        }

        info.details_available = true;
        std::string value;
        if (VxGetDeviceName(camera, value) == 0) {
            info.name = value;
        }
        value.clear();
        if (VxGetDevicePath(camera, value) == 0) {
            info.path = value;
        }
        value.clear();
        if (VxGetHardwareID(camera, value) == 0) {
            info.hardware_id = value;
        }
        value.clear();
        if (VxGetSensorUniqueID(camera, value) == 0) {
            info.serial_number = value;
        }

        VX_CAMERA_INTERFACE_TYPE interface_type{};
        if (VxGetDeviceInterfaceType(camera, interface_type) == 0) {
            info.interface_type = interface_type;
        }
        if (include_formats && VxGetFormatList(camera, info.formats) == 0) {
            info.formats_available = true;
        }

        VxClose(camera);
        devices.push_back(std::move(info));
    }

    return devices;
}

}  // namespace

int main(int argc, char** argv) {
    bool include_formats = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--formats") == 0 || std::strcmp(argv[i], "-f") == 0) {
            include_formats = true;
            continue;
        }
        std::cerr << "Usage: list_devices [--formats]\n";
        return 2;
    }

    const auto devices = DiscoverDevices(include_formats);
    std::cout << vizionsdk_ros2::FormatDeviceList(devices);
    return 0;
}
