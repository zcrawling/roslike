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
#include <stdexcept>
#include <zmq.hpp>


namespace Serializer {

    template<typename T>
    concept IsRange = std::ranges::range<T> && !std::is_same_v<std::remove_cvref_t<T>, std::string>;

    class Serializer {
    private:
        std::vector<uint8_t> buffer;
    public:
        auto data() const noexcept { return buffer.data(); }
        auto size() const noexcept { return buffer.size(); }
        //T(data type, 1byte) - V(value, N byte) if arithmetic
        //T(data type, 1byte) - L(length, 4byte) - V(value, N byte) if string
        //T(container type) - L(length of container, 4byte) - T(inner type)-(V,V,..) if container
        //int(n)_t, uint(n)_t, float, double,... (<30)

        template <typename T>
        requires std::is_arithmetic_v<T>
        void pack(T value, bool tag = true) {
            //T
            if (tag) {
                buffer.insert(buffer.end(), Datatype::get_tag<T>());
            }
            //V
            const auto* ptr = reinterpret_cast<const uint8_t*>(&value);
            buffer.insert(buffer.end(), ptr, ptr+sizeof(T));
        }

        //string (=30)
        template <typename T>
        requires std::is_convertible_v<T, std::string_view> &&
             (!std::is_arithmetic_v<std::remove_cvref_t<T>>)
        void pack(const T& value, bool tag = true) {
            const std::string_view sv = value;
            const auto len = static_cast<uint32_t>(sv.size());
            if (tag) buffer.push_back(ROSLIKE_STRING);
            const auto* len_ptr = reinterpret_cast<const uint8_t*>(&len);
            buffer.insert(buffer.end(), len_ptr, len_ptr + sizeof(uint32_t));
            buffer.insert(buffer.end(), sv.begin(), sv.end());
        }

        //pointer
        template<typename T>
        requires std::is_pointer_v<std::remove_cvref_t<T>> &&
             (!std::is_convertible_v<T, std::string_view>)
        void pack(T ptr, bool tag = true) {
            if (ptr == nullptr) throw std::invalid_argument("nullptr passed to pack()");
            pack(*ptr, tag);
        }

        //container
        template<IsRange T>
        requires IsRange<T> &&
             (!std::is_convertible_v<T, std::string_view>)
        void pack(const T& container, bool tag = true) {
            //T
            buffer.push_back(ROSLIKE_ARRAY);
            //L
            const auto len = static_cast<uint32_t>(std::size(container));
            const auto len_ptr = reinterpret_cast<const uint8_t*>(&len);
            buffer.insert(buffer.end(), len_ptr, len_ptr + sizeof(uint32_t));
            //T
            using ElementType = typename T::value_type;
            buffer.push_back(Datatype::get_tag<ElementType>());
            //V
            for (const auto& item : container) {
                if constexpr (std::is_arithmetic_v<ElementType>) {
                    const uint8_t* item_ptr = reinterpret_cast<const uint8_t*>(&item);
                    buffer.insert(buffer.end(), item_ptr, item_ptr + sizeof(ElementType));
                } else {
                    pack(item, false);
                }
            }
        }
        template<typename... Args>
        void serialize(Args&&... args) {
            (pack(std::forward<Args>(args)), ...);
        }
    };

    class Deserializer {
    private:
        std::vector<uint8_t> buffer;
        size_t offset = 0;

    public:
        template <typename T>
        void extract(uint8_t*& ptr, T& out, uint8_t tag = ROSLIKE_NONE) {
            if (tag == ROSLIKE_NONE) tag = *ptr++;

            if constexpr (std::is_arithmetic_v<T>) {
                if (tag >= ROSLIKE_STRING)
                    throw std::runtime_error("Tag mismatch: expected arithmetic type");
                std::memcpy(&out, ptr, sizeof(T));
                ptr += sizeof(T);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                if (tag != ROSLIKE_STRING)
                    throw std::runtime_error("Tag mismatch: expected STRING(30)");
                uint32_t len;
                std::memcpy(&len, ptr, 4);
                ptr += 4;
                out.assign(reinterpret_cast<const char*>(ptr), len);
                ptr += len;
            }
            else if constexpr (IsRange<T>) {
                if (tag != ROSLIKE_ARRAY)
                    throw std::runtime_error("Tag mismatch: expected ARRAY(40)");
                uint32_t len;
                std::memcpy(&len, ptr, 4);
                ptr += 4;
                uint8_t element_tag = *ptr++;
                out.resize(len);
                for (auto& item : out) {
                    extract(ptr, item, element_tag);
                }
            }
        }

        template<typename... Args>
        void unpack(Args&... args) {
            if (buffer.empty()) return;
            uint8_t* ptr = buffer.data();
            const uint8_t* end = ptr + buffer.size();
            ( (ptr < end ? extract(ptr, args) : void()), ... );
            offset = ptr - buffer.data();
        }
        void reset() {
            offset = 0;
            buffer.clear();
        }
        void set_buffer(std::vector<uint8_t> data) {
            buffer = std::move(data);
            offset = 0;
        }
    };
}

#endif //MY_ROS_SERIALIZER_H