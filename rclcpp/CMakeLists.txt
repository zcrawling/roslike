cmake_minimum_required(VERSION 4.1)
project(my_ros)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(cppzmq REQUIRED)

add_executable(my_ros main.cpp
        rclcpp.h
        datatype.h
        serializer.h
        #        test/serializer.cpp
)
target_compile_options(my_ros PRIVATE -freflection)
target_link_libraries(my_ros PRIVATE cppzmq-static)
