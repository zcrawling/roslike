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


//TODO() image-specified (de)serializer

namespace Serializer {
// #define PARSE_CASE(TAG, TYPE) \
// case TAG: { \
// TYPE val; std::memcpy(&val, ptr, sizeof(TYPE)); \
// ptr += sizeof(TYPE); \
// handle_value(val); \
// break; \
// }

    template<typename T>
    concept IsRange = std::ranges::range<T> && !std::is_same_v<std::remove_cvref_t<T>, std::string>;

    template<typename T>
    concept IsMap = IsRange<T> && requires { typename T::mapped_type; };

    class Serializer {
    private:
        std::vector<uint8_t> buffer;
    public:
        auto data() const noexcept { return buffer.data(); }
        auto size() const noexcept { return buffer.size(); }
        //T(data type, 1byte) - V(value, N byte) if arithmetic
        //T(data type, 1byte) - L(length, 4byte) - V(value, N byte) if string
        //T(container type) - L(length of container, 4byte) - ((T-V), (T-V), ...) if container
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
            const auto* len_ptr = reinterpret_cast<const uint8_t*>(&len);
            if (tag) {
                buffer.insert(buffer.end(), ROSLIKE_STRING);
            }
            buffer.insert(buffer.end(), len_ptr, len_ptr + sizeof(uint32_t));
            buffer.insert(buffer.end(), sv.begin(), sv.end());
        }

        //pointer
        template<typename T>
        requires std::is_pointer_v<std::remove_cvref_t<T>> &&
             (!std::is_convertible_v<T, std::string_view>)
        void pack(T ptr, bool tag = true) {
            if (ptr == nullptr) {
                std::cerr<<"nullptr inserted when packing";
                return ;
            }
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
            uint8_t element_tag = Datatype::get_tag<ElementType>();
            buffer.push_back(element_tag);
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
    public:
        std::vector<uint8_t> buffer;
        template <typename T>
        void extract(uint8_t*& ptr, T& out, uint8_t buffer_tag = ROSLIKE_NONE) {
            uint8_t raw_tag = (buffer_tag == 255) ? *ptr : buffer_tag;
            std::cout << "[Debug] Current Ptr: " << (void*)ptr
                      << " | Raw Tag: " << (int)raw_tag
                      << " | Type: " << typeid(T).name() << std::endl;
            if (buffer_tag == ROSLIKE_NONE) {
                buffer_tag = *ptr++;
            }
            if constexpr (std::is_arithmetic_v<T>) {
                assert(buffer_tag < ROSLIKE_STRING && "Error: Received arithmetic type");
                if (buffer_tag != ROSLIKE_BOOL) {
                    assert((buffer_tag % 10) == sizeof(T) && "Error: Byte size discrepancy");
                }
                else assert(sizeof(T) == sizeof(bool) && "Error: Received type: bool");
                std::memcpy(&out, ptr, sizeof(T));
                ptr += sizeof(T);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                assert(buffer_tag == 30 && "Tag mismatch: Expected String tag (31)");
                uint32_t len;
                std::memcpy(&len, ptr, 4);
                ptr += 4;
                out.assign(reinterpret_cast<const char*>(ptr), len);
                ptr += len;
            }
            else if constexpr (IsRange<T>) {
                assert(buffer_tag == 40 && "Tag mismatch: Expected Range tag (40)");
                uint32_t len;
                std::memcpy(&len, ptr, 4);
                ptr += 4;
                uint8_t element_tag = *ptr++;
                out.resize(len);
                for (auto& item : out) {
                    extract(ptr, item, element_tag);   // TODO() 성능 최적화 요소: TL-(TV,TV,...)>> TLT-(VV...)
                }
            }
        }

        template<typename... Args>
        void unpack(Args&... args) {
            if (buffer.empty()) return;
            uint8_t* ptr = buffer.data();
            const uint8_t* end = ptr + buffer.size();
            ( (ptr < end ? extract(ptr, args) : void()), ... );

            buffer.clear();
        }
    };
}

#endif //MY_ROS_SERIALIZER_H