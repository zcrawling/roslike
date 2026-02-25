#include "rclcpp.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    rclcpp::Node node("test_publisher");

    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        node.publish<int32_t, float, std::string, std::vector<int64_t>>(
            "sensor",
            i * 10,
            i * 1.5f,
            "hello_" + std::to_string(i),

        );
        std::cout << "[Pub] sent: " << i*10 << ", " << i*1.5f
                  << ", hello_" << i << ", [1,2,3]" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
