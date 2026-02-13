
#include <iostream>
#include "../rclcpp/serializer.h"
#include <string>


int main() {
    Serializer::Serializer se;
    Serializer::Deserializer de;
    std::string str = "123121caa아아아";
    std::string_view strv = "dandjk하하ㅏ하 123";
    std::vector<std::vector<std::string> > vec = {
        {"12", "하하", ""},
        {"123", "2333하", "ahhaha","   ","dasd"}
    };
    std::string a1;
    std::string_view a2;
    std::vector<std::vector<std::string>> s;
    uint8_t c;
    se.serialize(vec, str, 123);
    de.buffer.swap(se.buffer);
    de.unpack(s, a1, c);
    for (auto i : s) {
        for (auto j: i) {
            std::cout<<j<<" ";
        }
    }
    std::cout<<"\n"<<a1<<+c;
}
