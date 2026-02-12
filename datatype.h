//
// Created by sb on 2/12/26.
//

#ifndef MY_ROS_DATATYPE_H
#define MY_ROS_DATATYPE_H

#define ROSLIKE_NONE 0
#define ROSLIKE_BOOL 1
#define ROSLIKE_INT8 2
#define ROSLIKE_UINT8 3
#define ROSLIKE_INT16 4
#define ROSLIKE_UINT16 5
#define ROSLIKE_INT32 6
#define ROSLIKE_UINT32 7
#define ROSLIKE_INT64 8
#define ROSLIKE_UINT64 9
#define ROSLIKE_FLOAT32 11
#define ROSLIKE_FLOAT64 12
#define ROSLIKE_FLOAT128 13
#define ROSLIKE_STRING 20
#define ROSLIKE_ARRAY 30
#define ROSLIKE_ARRAY_START 31
#define ROSLIKE_ARRAY_END 32

#include <cstdint>
#include <type_traits>
#include <string>
//container & c-style array >> //TODO()
template<typename T>
consteval uint8_t get_tag() {
    if constexpr (std::is_same_v<T, bool>) return ROSLIKE_BOOL;
    else if constexpr (std::is_same_v<T, int8_t>) return ROSLIKE_INT8;
    else if constexpr (std::is_same_v<T, uint8_t>) return ROSLIKE_UINT8;
    else if constexpr (std::is_same_v<T, int16_t>) return ROSLIKE_INT16;
    else if constexpr (std::is_same_v<T, uint16_t>) return ROSLIKE_UINT16;
    else if constexpr (std::is_same_v<T, int32_t>) return ROSLIKE_INT32;
    else if constexpr (std::is_same_v<T, uint32_t>) return ROSLIKE_UINT32;
    else if constexpr (std::is_same_v<T, int64_t>) return ROSLIKE_INT64;
    else if constexpr (std::is_same_v<T, uint64_t>) return ROSLIKE_UINT64;
    else if constexpr (std::is_same_v<T, float>) return ROSLIKE_FLOAT32;
    else if constexpr (std::is_same_v<T, double>) return ROSLIKE_FLOAT64;
    else if constexpr (std::is_same_v<T, __float128>) return ROSLIKE_FLOAT128;
    else if constexpr (std::is_same_v<T, std::string>) return ROSLIKE_STRING;
    else if constexpr (std::is_same_v<T, std::string_view>) return ROSLIKE_STRING;
    //TODO()
    else return ROSLIKE_NONE;
}

#endif //MY_ROS_DATATYPE_H