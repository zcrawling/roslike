
#include "rclcpp.h"
#include <iostream>
#include <string>

void my_callback(uint8_t n, float f, std::string s,std::vector<std::vector<int64_t>> vec) {
    std::cout << "============================" << std::endl;
    std::cout << "Received Int: " << static_cast<int>(n) << std::endl;
    std::cout << "Received Float: " << f << std::endl;
    std::cout << "Received String: " << s << std::endl;
    for (auto i : vec) {
        for (auto j: i) {
        std::cout<<j<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout << "\n===========================" << std::endl;
}

int main() {
    rclcpp::Node node;
    std::cout<<sizeof(uint8_t)<<" "<<sizeof(float);
    // 1. 구독 신청 (콜백 등록)
    node.subscribe<uint8_t, float>("test_topic", std::function(my_callback));

    // 2. 파이썬 노드에 접속 (파이썬이 bind한 5555 포트로)
    rclcpp::Socket_info info;
    info.topic = "test_topic";
    info.addr = "tcp://127.0.0.1:5555";
    info.is_subscriber = true;

    node.update_socket(info);

    std::cout << "C++ Subscriber started. Waiting for data..." << std::endl;

    // 수신 스레드가 돌고 있으므로 메인 스레드는 대기
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}