//
// Created by sb on 2/12/26.
//

#ifndef MY_ROS_DATATYPE_H
#define MY_ROS_DATATYPE_H

#define ROSLIKE_NONE 0
#define ROSLIKE_BOOL 1
#define ROSLIKE_INT8 3
#define ROSLIKE_UINT8 4
#define ROSLIKE_INT16 6
#define ROSLIKE_UINT16 7
#define ROSLIKE_INT32 9
#define ROSLIKE_UINT32 10
#define ROSLIKE_INT64 12
#define ROSLIKE_UINT64 13
#define ROSLIKE_FLOAT32 20
#define ROSLIKE_FLOAT64 22
#define ROSLIKE_FLOAT128 24
#define ROSLIKE_STRING 30
#define ROSLIKE_ARRAY 40

#include <cstdint>
#include <type_traits>
#include <string>

namespace Datatype {
    template<typename T>
 uint8_t get_tag() {
        using rawT = std::conditional_t<
            std::is_pointer_v<std::remove_cvref_t<T>>,
            std::remove_pointer_t<std::remove_cvref_t<T>>,
        std::remove_cvref_t<T>
        >;
        if constexpr (std::is_same_v<rawT, bool>) return ROSLIKE_BOOL;
        else if constexpr (std::is_same_v<rawT, int8_t>) return ROSLIKE_INT8;
        else if constexpr (std::is_same_v<rawT, uint8_t>) return ROSLIKE_UINT8;
        else if constexpr (std::is_same_v<rawT, int16_t>) return ROSLIKE_INT16;
        else if constexpr (std::is_same_v<rawT, uint16_t>) return ROSLIKE_UINT16;
        else if constexpr (std::is_same_v<rawT, int32_t>) return ROSLIKE_INT32;
        else if constexpr (std::is_same_v<rawT, uint32_t>) return ROSLIKE_UINT32;
        else if constexpr (std::is_same_v<rawT, int64_t>) return ROSLIKE_INT64;
        else if constexpr (std::is_same_v<rawT, uint64_t>) return ROSLIKE_UINT64;
        else if constexpr (std::is_same_v<rawT, float>) return ROSLIKE_FLOAT32;
        else if constexpr (std::is_same_v<rawT, double>) return ROSLIKE_FLOAT64;
        else if constexpr (std::is_same_v<rawT, __float128>) return ROSLIKE_FLOAT128;
        else if constexpr (std::is_same_v<rawT, std::string>) return ROSLIKE_STRING;
        else if constexpr (std::is_same_v<rawT, std::string_view>) return ROSLIKE_STRING;
        //TODO() : NONE 예외처리
        else return ROSLIKE_NONE;
    }

}

#endif //MY_ROS_DATATYPE_H