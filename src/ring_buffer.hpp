// Lock-free ring buffer for single-producer/single-consumer usage.
// borrowed from https://github.com/51ddhesh/experiments/blob/main/C%2B%2B/RingBuffer/main.cpp

#pragma once

#include <vector>
#include <atomic>

// Lock-free ring buffer for thread-safe push/pop.
// T -> Element type.
template <typename T>
class RingBuffer {
private:
    const size_t size_;              // Buffer size (actual size + 1)
    std::vector<T> buffer_;          // Storage
    std::atomic<size_t> head_;       // Next push index
    std::atomic<size_t> tail_;       // Next pop index

public:
    // Construct ring buffer with given size.
    explicit RingBuffer(size_t size)
        : size_(size + 1), buffer_(size_), head_(0), tail_(0) {}
    
    // Push item. Returns false if full.
    bool push(const T& item) noexcept {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t next_head = (head + 1) % size_;
        
        // Buffer is full if next_head equals tail
        if (next_head == tail_.load(std::memory_order_acquire)) return false;
        
        buffer_[head] = item;
        head_.store(next_head, std::memory_order_release);
        
        return true;
    }

    // Pop item. Returns false if empty.
    bool pop(T& item) noexcept {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        
        // buffer is empty if tail equals head
        if (tail == head_.load(std::memory_order_acquire)) return false;
        
        item = buffer_[tail];
        tail_.store((tail + 1) % size_, std::memory_order_release);
        
        return true;
    }
};

