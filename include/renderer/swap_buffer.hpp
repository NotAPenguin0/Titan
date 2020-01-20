#ifndef TITAN_TERRAIN_RENDERER_SWAP_BUFFER_HPP_
#define TITAN_TERRAIN_RENDERER_SWAP_BUFFER_HPP_

#include <thread>

namespace titan::renderer {

class SwapBuffer {
public:
    SwapBuffer() = default;
    SwapBuffer(SwapBuffer&&);

    SwapBuffer& operator=(SwapBuffer&&);

    void create(unsigned int buffer_target, size_t max_byte_size);

    ~SwapBuffer();

    unsigned int get() const;

    size_t current_size() const;
    size_t max_size() const;

    // The data pointer must be valid for the entire duration of the data upload, e.g. until wait_for_upload() is called
    void start_data_upload(void const* data, size_t len);
    // This function does not return until the data upload is complete, and then flushes the changes to the GPU
    void wait_for_upload();

    void swap(SwapBuffer& other);

private:
    unsigned int target = 0;
    unsigned int handle = 0;
    size_t size = 0;

    void* mapped_data = nullptr;
    size_t cur_write_length = 0;

    std::thread worker;
};

}


#endif