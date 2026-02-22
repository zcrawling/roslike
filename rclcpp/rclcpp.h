//
// Created by sb on 2/12/26.
//

#ifndef MY_ROS_RCLCPP_H
#define MY_ROS_RCLCPP_H

#include "serializer.h"
#include <unordered_map>
#include <BS_thread_pool.hpp>
#include <functional>

namespace rclcpp {

    struct Socket_info {
        std::string topic;
        std::string addr;
        bool is_subscriber;
    };
    class Node {
    private:
        zmq::context_t context;
        std::unordered_map<std::string, std::unique_ptr<zmq::socket_t>> pub_sockets;
        std::unordered_map<std::string, std::unique_ptr<zmq::socket_t>> sub_sockets;
        std::unordered_map<std::string, std::function<void(const std::vector<uint8_t>&)>> callbacks;

        BS::thread_pool<> pool;
        std::atomic<bool> running{true};
        std::thread receiver_thread;

    public:
        Node() : context(1), pool(std::thread::hardware_concurrency()) {
            receiver_thread = std::thread([this] { run_receiver(); });
        }

        ~Node() {
            running = false;
            if (receiver_thread.joinable()) receiver_thread.join();
        }

        template<typename... Args>
        void subscribe(const std::string& topic, std::function<void(Args...)> callback) {
            if (!sub_sockets.contains(topic)) {
                sub_sockets[topic] = std::make_unique<zmq::socket_t>(context, ZMQ_SUB);
                sub_sockets[topic]->set(zmq::sockopt::subscribe, "");
                // Discovery에서 연결해주기 전까진 일단 소켓만 생성 TODO() master node
            }
            callbacks[topic] = [this, callback](const std::vector<uint8_t>& data) {
                Serializer::Deserializer de;
                de.buffer = data;

                std::tuple<std::decay_t<Args>...> args_tuple;

                std::apply([&de](auto&... args) {
                    de.unpack(args...);
                }, args_tuple);

                std::apply(callback, args_tuple);
            };
        }

        template<typename... Args>
        void publish(const std::string& topic, Args&&... args) {
            if (!pub_sockets.contains(topic)) {
                pub_sockets[topic] = std::make_unique<zmq::socket_t>(context, ZMQ_PUB);
            }

            Serializer::Serializer se;
            (se.serialize(std::forward<Args>(args)), ...);

            zmq::message_t msg(se.data(), se.size());
            pub_sockets[topic]->send(msg, zmq::send_flags::none);
        }

        void update_socket(const Socket_info& info) {
            if (info.is_subscriber) {
                if (sub_sockets.contains(info.topic)) {
                    sub_sockets[info.topic]->set(zmq::sockopt::subscribe, "");
                    sub_sockets[info.topic]->connect(info.addr);
                }
            } else {
                if (pub_sockets.contains(info.topic)) {
                    pub_sockets[info.topic]->connect(info.addr);
                }
            }
        }

    private:
        void run_receiver() {
            while (running) {
                for (auto& [topic, socket] : sub_sockets) {
                    zmq::message_t msg;
                    if (socket->recv(msg, zmq::recv_flags::dontwait)) { //TODO() policy
                        std::vector<uint8_t> data(
                            static_cast<uint8_t*>(msg.data()),
                            static_cast<uint8_t*>(msg.data()) + msg.size()
                        );

                        if (callbacks.contains(topic)) {
                            pool.detach_task([cb = callbacks[topic], d = std::move(data)]() {
                                cb(d);
                            });
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::microseconds(50000)); //TODO() policy
            }
        }
    };
}



#endif //MY_ROS_RCLCPP_H