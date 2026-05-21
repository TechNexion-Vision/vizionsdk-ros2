#include "vizionsdk_ros2/vizionsdk_camera_node.hpp"

#include <rclcpp/rclcpp.hpp>

#include <exception>
#include <iostream>
#include <memory>

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);

    try {
        auto node = std::make_shared<vizionsdk_ros2::VizionSdkCameraNode>();
        rclcpp::spin(node);
    } catch (const std::exception& ex) {
        std::cerr << "vizionsdk_camera_node failed: " << ex.what() << '\n';
        rclcpp::shutdown();
        return 1;
    }

    rclcpp::shutdown();
    return 0;
}
