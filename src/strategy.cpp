#include "./events.hpp"   
#include "./ring_buffer.hpp"  
#include "./logger.hpp"
#include <chrono>
#include <thread>
#include <stop_token>

// External counters for order creation and position tracking
extern std::atomic<int> orders_created;
extern std::atomic<int> position;
extern std::atomic<double> avg_cost;

// Strategy parameters
const double BUY_THRESHOLD = 100.4;
const double SELL_THRESHOLD = 100.7;
const int MAX_POSITION = 10000;

// Consumes market data and generates buy/sell orders based on thresholds
void run_strategy(std::stop_token st, RingBuffer<MarketDataEvent>& input, RingBuffer<OrderEvent>& output) {
    while (!st.stop_requested()) {
        MarketDataEvent event;
        // Try to pop a market event from the buffer
        if (input.pop(event)) {
            int current_pos = position.load();

            // Buy if price below threshold and position limit not reached
            if (event.price < BUY_THRESHOLD && current_pos < MAX_POSITION) {
                int max_qty = MAX_POSITION - current_pos;
                int qty = std::min(event.quantity, max_qty);

                OrderEvent order{
                    .timestamp = event.timestamp,
                    .price = event.price,
                    .quantity = qty,
                    .side = 'B'
                };

                // Push buy order and log
                if (output.push(order)) {
                    ++orders_created;
                    Logger::log("  [Strategy] Buy @ $", order.price, " x", order.quantity);
                }
            }

            // Sell if price above threshold and position available
            else if (event.price > SELL_THRESHOLD && current_pos > 0) {
                int qty = std::min(event.quantity, current_pos);

                OrderEvent order{
                    .timestamp = event.timestamp,
                    .price = event.price,
                    .quantity = qty,
                    .side = 'S'
                };

                // Push sell order and log
                if (output.push(order)) {
                    ++orders_created;
                    Logger::log("  [Strategy] Sell @ $", order.price, " x", order.quantity);
                }
            }

        } else {
            // Sleep briefly if no event is available
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
}

