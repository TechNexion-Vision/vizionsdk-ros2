#pragma once

#include "VxPublicTypes.hpp"

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace vizionsdk_ros2 {

struct DeviceInfo {
    int index{};
    std::string name;
    std::string path;
    std::optional<VX_CAMERA_INTERFACE_TYPE> interface_type;
    std::string hardware_id;
    std::string serial_number;
    bool details_available{};
};

inline std::string InterfaceTypeName(VX_CAMERA_INTERFACE_TYPE type) {
    switch (type) {
        case VX_CAMERA_INTERFACE_TYPE::INTERFACE_USB:
            return "USB";
        case VX_CAMERA_INTERFACE_TYPE::INTERFACE_MIPI_CSI2:
            return "MIPI_CSI2";
        case VX_CAMERA_INTERFACE_TYPE::INTERFACE_ETHERNET:
            return "ETHERNET";
        default:
            return "UNKNOWN";
    }
}

inline std::string ValueOrUnknown(const std::string& value) {
    return value.empty() ? "unknown" : value;
}

inline std::string FormatDeviceList(const std::vector<DeviceInfo>& devices) {
    if (devices.empty()) {
        return "No Vizion camera devices found.\n";
    }

    std::ostringstream oss;
    for (const auto& device : devices) {
        oss << "[" << device.index << "] " << ValueOrUnknown(device.name) << "\n";
        oss << "    path: " << ValueOrUnknown(device.path) << "\n";
        oss << "    interface: "
            << (device.interface_type.has_value() ? InterfaceTypeName(*device.interface_type)
                                                   : "unknown")
            << "\n";
        oss << "    hardware_id: " << ValueOrUnknown(device.hardware_id) << "\n";
        oss << "    serial: " << ValueOrUnknown(device.serial_number) << "\n";
        if (!device.details_available) {
            oss << "    warning: failed to open device details\n";
        }
    }
    return oss.str();
}

}  // namespace vizionsdk_ros2
