#include "./events.hpp"
#include "./ring_buffer.hpp"
#include "./logger.hpp"
#include <atomic>
#include <thread>
#include <stop_token>

// Function declarations
void run_feed_handler(std::stop_token st, RingBuffer<MarketDataEvent>& buffer);
void run_strategy(std::stop_token st, RingBuffer<MarketDataEvent>& input, RingBuffer<OrderEvent>& output);
void run_order_executor(std::stop_token st, RingBuffer<OrderEvent>& buffer);

// Global Counters
std::atomic<int> market_events_generated{0};
std::atomic<int> orders_created{0};
std::atomic<int> orders_executed{0};

// Profit tracking
std::atomic<double> total_spent{0.0};
std::atomic<double> total_earned{0.0};

std::atomic<int> position{0};         // Net quantity owned
std::atomic<double> avg_cost{0.0};    // Average price of current position
std::atomic<int> total_quantity_bought{0};


int main() {
    RingBuffer<MarketDataEvent> market_data_buffer(1024);
    RingBuffer<OrderEvent> order_buffer(1024);

    std::jthread feed_thread(run_feed_handler, std::ref(market_data_buffer));
    std::jthread strategy_thread(run_strategy, std::ref(market_data_buffer), std::ref(order_buffer));
    std::jthread executor_thread(run_order_executor, std::ref(order_buffer));

    std::this_thread::sleep_for(std::chrono::seconds(3));  // Run for 3 seconds

    // Threads automatically receive stop request on destruction

    Logger::log("\n=== Summary Start ===");
    Logger::log("Market Events:   ", market_events_generated.load());
    
    Logger::log("Orders Created:  ", orders_created.load());
    Logger::log("Orders Executed: ",orders_executed.load());
    
    Logger::log("Total Spent:     $", total_spent.load());
    Logger::log("Total Earned:     $", total_earned.load());

    Logger::log("Total Quantity Bought: ", total_quantity_bought.load());
    Logger::log("Open Position:         ", position.load());
    Logger::log("Avg Cost of Position:  $", avg_cost.load());
    Logger::log("Net Profit:            $", total_earned.load() - total_spent.load());
    Logger::log("\n=== Summary End ===");
    return 0;
}
