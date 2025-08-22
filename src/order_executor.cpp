#include "./events.hpp"
#include "./ring_buffer.hpp"
#include "./logger.hpp"
#include <thread>
#include <stop_token>

// External counters for tracking order execution and position
extern std::atomic<int> orders_executed;
extern std::atomic<double> total_spent;
extern std::atomic<double> total_earned;
extern std::atomic<int> position;
extern std::atomic<double> avg_cost;
extern std::atomic<int> total_quantity_bought;

// Executes orders from the buffer and updates trading statistics
void run_order_executor(std::stop_token st, RingBuffer<OrderEvent>& buffer) {
    while (!st.stop_requested()) {
        OrderEvent order;
        // Try to pop an order from the buffer
        if (buffer.pop(order)) {
            ++orders_executed;
            double value = order.price * order.quantity;

            if (order.side == 'B') {
                total_spent += value;
                total_quantity_bought += order.quantity;

                // Update position and average cost
                int pos = position.fetch_add(order.quantity);
                double new_avg = ((avg_cost * pos) + value) / (pos + order.quantity);
                avg_cost.store(new_avg);

            } else if (order.side == 'S') {
                total_earned += value;
                position.fetch_sub(order.quantity);
            }

            // Log execution details
            Logger::log("    [Executor] Executed ", order.side == 'B' ? "BUY" : "SELL",
                        " @ $", order.price, " x", order.quantity);
        } else {
            // Sleep briefly if no order is available
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
}
