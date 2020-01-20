#include "renderer/swap_buffer.hpp"

#include <glad/glad.h>

#include <iostream>

namespace titan::renderer {

SwapBuffer::SwapBuffer(SwapBuffer&& rhs) {
    std::swap(size, rhs.size);
    std::swap(handle, rhs.handle);
    std::swap(worker, rhs.worker);
    std::swap(target, rhs.target);
    std::swap(mapped_data, rhs.mapped_data);
    std::swap(cur_write_length, rhs.cur_write_length);
}

SwapBuffer& SwapBuffer::operator=(SwapBuffer&& rhs) {
    std::swap(size, rhs.size);
    std::swap(handle, rhs.handle);
    std::swap(worker, rhs.worker);
    std::swap(target, rhs.target);
    std::swap(mapped_data, rhs.mapped_data);
    std::swap(cur_write_length, rhs.cur_write_length);
    return *this;
}

void SwapBuffer::create(unsigned int buffer_target, size_t max_byte_size) {
    target = buffer_target;
    size = max_byte_size;

    glGenBuffers(1, &handle);
    // Set buffer size by uploading null as data
    constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
    glBindBuffer(target, handle);
    glBufferStorage(target, max_byte_size, nullptr, flags);
    // Grab data pointer to persistent buffer storage
    mapped_data = glMapBufferRange(target, 0, max_byte_size, flags | GL_MAP_FLUSH_EXPLICIT_BIT);
}

SwapBuffer::~SwapBuffer() {
    glBindBuffer(target, handle);
    glUnmapBuffer(target);
    glDeleteBuffers(1, &handle);
}

unsigned int SwapBuffer::get() const {
    return handle;
}

size_t SwapBuffer::current_size() const {
    return cur_write_length;
}

size_t SwapBuffer::max_size() const {
    return size;
}

void SwapBuffer::start_data_upload(void const* data, size_t len) {
    cur_write_length = len;
    worker = std::move(std::thread(
        std::memcpy, mapped_data, data, len
    ));
}

void SwapBuffer::wait_for_upload() {
    if (worker.joinable()) {
        worker.join();
        glBindBuffer(target, handle);
        glFlushMappedBufferRange(target, 0, cur_write_length);
    } else { 
        return; 
    }
}

void SwapBuffer::swap(SwapBuffer& rhs) {
    std::swap(size, rhs.size);
    std::swap(handle, rhs.handle);
    std::swap(worker, rhs.worker);
    std::swap(target, rhs.target);
    std::swap(mapped_data, rhs.mapped_data);
    std::swap(cur_write_length, rhs.cur_write_length);

}

}