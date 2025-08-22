#include "./events.hpp"   
#include "./ring_buffer.hpp"  
#include "./logger.hpp"
#include <chrono>
#include <thread>
#include <stop_token>
#include <deque>
#include <numeric>

// External counters for order creation and position tracking
extern std::atomic<int> orders_created;
extern std::atomic<int> position;
extern std::atomic<double> avg_cost;

// Enhanced strategy parameters
const int LOOKBACK_PERIOD = 20;        // Number of prices to track for moving average
const double VOLATILITY_THRESHOLD = 0.15; // Minimum deviation from MA to trade
const int MAX_POSITION = 5000;         // Reduced position limit
const int ORDER_SIZE = 100;            // Fixed order size
const double TAKE_PROFIT_PCT = 0.002;  // 0.2% take profit
const double STOP_LOSS_PCT = 0.001;    // 0.1% stop loss

// Strategy state
class StrategyState {
public:
    std::deque<double> price_history;
    double moving_average = 100.0;
    double last_buy_price = 0.0;
    double last_sell_price = 0.0;
    
    void update_moving_average(double new_price) {
        price_history.push_back(new_price);
        if (price_history.size() > LOOKBACK_PERIOD) {
            price_history.pop_front();
        }
        
        if (!price_history.empty()) {
            moving_average = std::accumulate(price_history.begin(), 
                                           price_history.end(), 0.0) / price_history.size();
        }
    }
    
    bool has_enough_history() const {
        return price_history.size() >= LOOKBACK_PERIOD;
    }
};

// Mean reversion strategy with improved risk management
void run_strategy(std::stop_token st, RingBuffer<MarketDataEvent>& input, RingBuffer<OrderEvent>& output) {
    StrategyState state;
    
    while (!st.stop_requested()) {
        MarketDataEvent event;
        
        if (input.pop(event)) {
            // Update moving average
            state.update_moving_average(event.price);
            
            // Skip trading until we have enough history
            if (!state.has_enough_history()) {
                continue;
            }
            
            int current_pos = position.load();
            double deviation = event.price - state.moving_average;
            double abs_deviation = std::abs(deviation);
            
            // Only trade if deviation is significant enough
            if (abs_deviation < VOLATILITY_THRESHOLD) {
                continue;
            }
            
            // MEAN REVERSION LOGIC:
            // Buy when price is significantly BELOW moving average (oversold)
            if (deviation < -VOLATILITY_THRESHOLD && current_pos < MAX_POSITION) {
                int max_qty = MAX_POSITION - current_pos;
                int qty = std::min(ORDER_SIZE, max_qty);
                
                OrderEvent order{
                    .timestamp = event.timestamp,
                    .price = event.price,
                    .quantity = qty,
                    .side = 'B'
                };
                
                if (output.push(order)) {
                    ++orders_created;
                    state.last_buy_price = event.price;
                    Logger::log("  [Strategy] Buy @ $", order.price, " x", order.quantity, 
                               " (MA: ", state.moving_average, ", Dev: ", deviation, ")");
                }
            }
            
            // Sell when price is significantly ABOVE moving average (overbought)
            else if (deviation > VOLATILITY_THRESHOLD && current_pos > 0) {
                int qty = std::min(ORDER_SIZE, current_pos);
                
                OrderEvent order{
                    .timestamp = event.timestamp,
                    .price = event.price,
                    .quantity = qty,
                    .side = 'S'
                };
                
                if (output.push(order)) {
                    ++orders_created;
                    state.last_sell_price = event.price;
                    Logger::log("  [Strategy] Sell @ $", order.price, " x", order.quantity,
                               " (MA: ", state.moving_average, ", Dev: ", deviation, ")");
                }
            }
            
            // RISK MANAGEMENT: Take profit/stop loss on existing positions
            else if (current_pos > 0 && state.last_buy_price > 0) {
                double profit_pct = (event.price - state.last_buy_price) / state.last_buy_price;
                
                // Take profit or stop loss
                if (profit_pct > TAKE_PROFIT_PCT || profit_pct < -STOP_LOSS_PCT) {
                    int qty = std::min(ORDER_SIZE, current_pos);
                    
                    OrderEvent order{
                        .timestamp = event.timestamp,
                        .price = event.price,
                        .quantity = qty,
                        .side = 'S'
                    };
                    
                    if (output.push(order)) {
                        ++orders_created;
                        const char* reason = (profit_pct > TAKE_PROFIT_PCT) ? "TAKE_PROFIT" : "STOP_LOSS";
                        Logger::log("  [Strategy] ", reason, " Sell @ $", order.price, 
                                   " x", order.quantity, " (P&L: ", profit_pct * 100, "%)");
                    }
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
}