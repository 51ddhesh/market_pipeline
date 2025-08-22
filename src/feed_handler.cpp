#include "./events.hpp" 
#include "./ring_buffer.hpp"
#include "./logger.hpp"
#include <random>
#include <thread>
#include <chrono>
#include <stop_token>

// External counter for tracking generated market events
extern std::atomic<int> market_events_generated;

// Simulates a market data feed and pushes events to the ring buffer
void run_feed_handler(std::stop_token st, RingBuffer<MarketDataEvent>& buffer) {
    std::mt19937 rng(std::random_device{}()); // Random number generator
    std::uniform_real_distribution<double> price_distribution(99.5, 101.0); // Price range
    std::uniform_int_distribution<int> quantity_dist(1, 10); // Quantity range

    while (!st.stop_requested()) {
        // Create a random market data event
        MarketDataEvent event {
            .timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count(),
            .price = price_distribution(rng),
            .quantity = quantity_dist(rng),
            .side = 'B'
        };

        // Push event to buffer and log if successful
        if (buffer.push(event)) {
            ++market_events_generated;
            Logger::log("[Feed] Event: $", event.price);
        }
   
        std::this_thread::sleep_for(std::chrono::microseconds(100)); // Simulate feed rate
    }
}
