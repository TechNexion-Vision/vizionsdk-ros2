#include "vizionsdk_ros2/yuv422_converter.hpp"

#include <memory>
#include <string>
#include <utility>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace {

class Yuv422ToBgrNode final : public rclcpp::Node {
   public:
    Yuv422ToBgrNode() : Node("yuv422_to_bgr") {
        const auto input_topic = declare_parameter<std::string>("input_topic", "image_raw");
        const auto output_topic = declare_parameter<std::string>("output_topic", "image_raw/bgr");
        output_frame_id_ = declare_parameter<std::string>("output_frame_id", "");

        publisher_ = create_publisher<sensor_msgs::msg::Image>(output_topic, rclcpp::SensorDataQoS());
        subscription_ = create_subscription<sensor_msgs::msg::Image>(
            input_topic, rclcpp::SensorDataQoS(),
            [this](sensor_msgs::msg::Image::ConstSharedPtr msg) { ConvertAndPublish(std::move(msg)); });

        RCLCPP_INFO(get_logger(), "Converting %s to %s as bgr8", input_topic.c_str(),
                    output_topic.c_str());
    }

   private:
    void ConvertAndPublish(const sensor_msgs::msg::Image::ConstSharedPtr& input) {
        if (!vizionsdk_ros2::IsSupportedYuv422Encoding(input->encoding)) {
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000,
                                 "Unsupported image encoding '%s'; expected uyvy or yuv422_yuy2",
                                 input->encoding.c_str());
            return;
        }

        try {
            sensor_msgs::msg::Image output;
            output.header = input->header;
            if (!output_frame_id_.empty()) {
                output.header.frame_id = output_frame_id_;
            }
            output.height = input->height;
            output.width = input->width;
            output.encoding = "bgr8";
            output.is_bigendian = false;
            output.step = input->width * 3U;
            output.data = vizionsdk_ros2::ConvertYuv422ToBgr(input->data, input->width,
                                                             input->height, input->encoding);
            publisher_->publish(std::move(output));
        } catch (const std::exception& error) {
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000,
                                 "Failed to convert YUV422 image: %s", error.what());
        }
    }

    std::string output_frame_id_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
};

}  // namespace

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Yuv422ToBgrNode>());
    rclcpp::shutdown();
    return 0;
}
