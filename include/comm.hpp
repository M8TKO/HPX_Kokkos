#pragma once
#include <cstddef>
#include <string>
#include <type_traits>

// Runtime hooks (implemented in gpu/comm_gpu.cpp)
void comm_runtime_init(int& argc, char**& argv);
void comm_runtime_finalize();

class communicator {
    struct impl;          // forward declaration, impl = implementation
    impl* p = nullptr;    // opaque pointer

public:
    communicator(std::string s = "Cuda");                    
    explicit communicator(std::size_t n, std::string s);
    ~communicator();

    communicator(const communicator&) = delete;
    communicator& operator=(const communicator&) = delete;
    

    void resize(std::size_t n);
    void print() const;
};
