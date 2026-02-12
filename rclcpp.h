//
// Created by sb on 2/12/26.
//

#ifndef MY_ROS_RCLCPP_H
#define MY_ROS_RCLCPP_H

#include "datatype.h"

#include <string>
#include <vector>
#include <type_traits>
#include <iostream>

#include <zmq.hpp>
#include <nlohmann/json.hpp>


namespace rclcpp {
    class Publisher {

    };

    class Subscription {

    };

    class Node {
    private:
        Publisher __pub__;
        Subscription __sub__;
    };

    class Client {

    };

    class Service {

    };
    class Serializer {
    private:
        std::vector<uint8_t> buffer;
    public:
        //int(n)_t, uint(n)_t, float, double,...
        template <typename T>
        requires std::is_arithmetic_v<T>
        void pack(T value) {
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
            buffer.insert(buffer.end(), ptr, ptr+sizeof(T));
        }
        //string
        void pack(const std::string & value) {
            uint32_t len = static_cast<uint32_t>(value.size());
            pack(len);
            buffer.insert(buffer.end(), value.begin(), value.end());
        }
        //pointer
        template<typename T>
        requires std::is_pointer_v<T>
        void pack(T ptr) {
            if (ptr == nullptr) {
                std::cerr<<"nullptr inserted when packing";
                return ;
            }
            pack(*ptr);
        }

        //recursive call when variable args
        template<typename... Args>
        void process(Args&&... args) {
            (pack(std::forward<Args>(args)), ...);
        }
        void serialize() {

        }
        void deserialize();
    };
    class DDS {

    };




}


#endif //MY_ROS_RCLCPP_H