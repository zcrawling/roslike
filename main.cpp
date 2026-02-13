
#include <iostream>
#include "rclcpp.h"
#include <string>
#include "serializer.h"

int main() {
    Serializer::Serializer se;
    Serializer::Deserializer de;
    std::string str = "123121caa아아아";
    std::string_view strv = "dandjk하하ㅏ하 123";
    std::vector<int16_t> vec;
    vec.push_back(1);
    vec.push_back(-155);
    vec.push_back(2312120);
    se.pack(vec);
    de.buffer.swap(se.buffer);
    de.deserialize();
}
