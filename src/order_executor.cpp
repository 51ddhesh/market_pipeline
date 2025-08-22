#include "./events.hpp"
#include "./ring_buffer.hpp"
#include "./logger.hpp"
#include <thread>
#include <stop_token>
#include <mutex>

// External counters for tracking order execution and position
extern std::atomic<int> orders_executed;
extern std::atomic<double> total_spent;
extern std::atomic<double> total_earned;
extern std::atomic<int> position;
extern std::atomic<double> avg_cost;
extern std::atomic<int> total_quantity_bought;

// Mutex for thread-safe position updates
std::mutex position_mutex;

// Executes orders from the buffer and updates trading statistics
void run_order_executor(std::stop_token st, RingBuffer<OrderEvent>& buffer) {
    while (!st.stop_requested()) {
        OrderEvent order;
        
        if (buffer.pop(order)) {
            ++orders_executed;
            double value = order.price * order.quantity;
            
            // Thread-safe position management
            std::lock_guard<std::mutex> lock(position_mutex);
            
            if (order.side == 'B') {
                total_spent += value;
                total_quantity_bought += order.quantity;
                
                // Update position and average cost atomically
                int old_pos = position.load();
                double old_avg = avg_cost.load();
                
                int new_pos = old_pos + order.quantity;
                double new_avg = old_pos == 0 ? order.price : 
                                ((old_avg * old_pos) + value) / new_pos;
                
                position.store(new_pos);
                avg_cost.store(new_avg);
                
            } else if (order.side == 'S') {
                total_earned += value;
                int old_pos = position.load();
                position.store(old_pos - order.quantity);
                
                // Reset average cost if position becomes zero
                if (position.load() <= 0) {
                    avg_cost.store(0.0);
                }
            }
            
            Logger::log("    [Executor] Executed ", order.side == 'B' ? "BUY" : "SELL",
                        " @ $", order.price, " x", order.quantity, 
                        " (Pos: ", position.load(), ")");
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
}