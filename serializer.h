//
// Created by sb on 2/13/26.
//

#ifndef MY_ROS_SERIALIZER_H
#define MY_ROS_SERIALIZER_H


#include "datatype.h"
#include <string>
#include <vector>
#include <ranges>
#include <type_traits>
#include <iostream>
#include <zmq.hpp>
#include <nlohmann/json.hpp>

namespace Serializer {
#define PARSE_CASE(TAG, TYPE) \
case TAG: { \
TYPE val; std::memcpy(&val, ptr, sizeof(TYPE)); \
ptr += sizeof(TYPE); \
handle_value(val); \
break; \
}

    template<typename T>
    concept IsRange = std::ranges::range<T> && !std::is_same_v<std::remove_cvref_t<T>, std::string>;

    template<typename T>
    concept IsMap = IsRange<T> && requires { typename T::mapped_type; };

    class Serializer {
    public:
        //T(data type, 1byte) - V(value, N byte) if arithmetic
        //T(data type, 1byte) - L(length, 4byte) - V(value, N byte) if string
        //T(container type) - L(length of container, 4byte) - ((T-V), (T-V), ...) if container
        std::vector<uint8_t> buffer;

    public:
        //int(n)_t, uint(n)_t, float, double,... (<30)
        template <typename T>
        requires std::is_arithmetic_v<T>
        void pack(T value) {
            const auto* ptr = reinterpret_cast<const uint8_t*>(&value);
            buffer.insert(buffer.end(), Datatype::get_tag<T>());
            buffer.insert(buffer.end(), ptr, ptr+sizeof(T));
        }
        //string (=30)
        template <typename T>
        requires std::is_convertible_v<T, std::string_view> &&
             (!std::is_arithmetic_v<std::remove_cvref_t<T>>)
        void pack(const T& value) {
            const std::string_view sv = value;
            const auto len = static_cast<uint32_t>(sv.size());
            const auto* len_ptr = reinterpret_cast<const uint8_t*>(&len);
            buffer.insert(buffer.end(), ROSLIKE_STRING);
            buffer.insert(buffer.end(), len_ptr, len_ptr + sizeof(uint32_t));
            buffer.insert(buffer.end(), sv.begin(), sv.end());
        }

        //pointer
        template<typename T>
        requires std::is_pointer_v<std::remove_cvref_t<T>> &&
             (!std::is_convertible_v<T, std::string_view>)
        void pack(T ptr) {
            if (ptr == nullptr) {
                std::cerr<<"nullptr inserted when packing";
                return ;
            }
            pack(*ptr);
        }
        //container
        template<IsRange T>
        requires IsRange<T> &&
             (!std::is_convertible_v<T, std::string_view>)
        void pack(const T& container) {
            const auto len = static_cast<uint32_t>(std::size(container));
            const auto len_ptr = reinterpret_cast<const uint8_t*>(&len);
            //T-L-V  (T-L- (T-V, T-V, ...))
            buffer.push_back(ROSLIKE_ARRAY);
            buffer.insert(buffer.end(), len_ptr, len_ptr + sizeof(uint32_t));
            for (const auto& item : container) {
                pack(item);
            }
        }
        //recursive call when variable args
        template<typename... Args>
        void process(Args&&... args) {
            (pack(std::forward<Args>(args)), ...);
        }
        void serialize() {

        }
    };

    class Deserializer {
    public:
        std::vector<uint8_t> buffer;
        template <typename T>
        static void handle_value(T val) {
            std::cout<< val<< std::endl;
        }
        static void handle_value(__float128 val) {
            std::cout<<"float 128"<<std::endl;
        }
        void parsing(uint8_t*& ptr, uint8_t type) {
            switch (type) {
                PARSE_CASE(ROSLIKE_BOOL, bool);
                PARSE_CASE(ROSLIKE_INT8, int8_t);
                PARSE_CASE(ROSLIKE_UINT8, uint8_t);
                PARSE_CASE(ROSLIKE_INT16, int16_t);
                PARSE_CASE(ROSLIKE_UINT16, uint16_t);
                PARSE_CASE(ROSLIKE_INT32, int32_t);
                PARSE_CASE(ROSLIKE_UINT32, uint32_t);
                PARSE_CASE(ROSLIKE_INT64, int64_t);
                PARSE_CASE(ROSLIKE_UINT64, uint64_t);
                PARSE_CASE(ROSLIKE_FLOAT32, float);
                PARSE_CASE(ROSLIKE_FLOAT64, double);
                PARSE_CASE(ROSLIKE_FLOAT128, __float128);
                case(ROSLIKE_STRING): {
                    uint32_t len;
                    std::memcpy(&len, ptr, 4);
                    ptr +=4;
                    std::string str(reinterpret_cast<const char*>(ptr), len);
                    ptr+=len;
                    handle_value(str);
                    break;
                }
                case(ROSLIKE_ARRAY): {
                    uint8_t inner_type;
                    uint32_t len;
                    std::memcpy(&len, ptr, sizeof(uint32_t));
                    ptr += sizeof(uint32_t);
                    for (uint32_t i = 0; i < len; ++i) {
                        inner_type = *ptr++;
                        parsing(ptr, inner_type);
                        std::cout<<"-------\n";
                    }
                    break;
                }
                default: ;
                //컨테이너 안에 string있는경우 고려
                //default:
            }
        }
public:
    void deserialize() {
        auto ptr = buffer.data();
        const uint8_t* end = ptr + buffer.size();
        if (ptr == nullptr) {
            return;
        }
        while (ptr < end) {
            uint8_t type = *ptr++;
            parsing(ptr, type);
        }
        buffer.clear();
    }
};


}

#endif //MY_ROS_SERIALIZER_H