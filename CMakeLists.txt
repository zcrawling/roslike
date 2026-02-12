cmake_minimum_required(VERSION 4.1)
project(my_ros)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # gnu++ 아닌 순수 c++26

find_package(cppzmq REQUIRED)

add_executable(my_ros main.cpp
        rclcpp.cpp
        rclcpp.h
        shared_mem.cpp
        shared_mem.h
        datatype.h
)
target_compile_options(my_ros PRIVATE -freflection)
target_link_libraries(my_ros PRIVATE cppzmq-static)
