//
// Created by sb on 2/12/26.
//

#ifndef MY_ROS_SHAREDM_H
#define MY_ROS_SHAREDM_H

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

class shared_mem {
public:
    static void* create_shm(const std::string& name, size_t size) {
        const int fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        ftruncate(fd, size);
        void* ptr = mmap(nullptr, size, PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
        return ptr;
    }

    void* attach_shm(const std::string& name, size_t size) {
        int fd = shm_open(name.c_str(), O_RDONLY, 0666);
        if (fd == -1) {
            return nullptr;
        }
        void* ptr = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
        close(fd);
        if (ptr == MAP_FAILED) return nullptr;
        return ptr;
    }

    void detach(void* ptr, size_t size) {
        if (ptr != nullptr) {
            munmap(ptr, size);
        }
    }
};


#endif //MY_ROS_SHAREDM_H