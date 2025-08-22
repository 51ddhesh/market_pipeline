# HFT Pipeline

A high-frequency trading (HFT) pipeline simulation in C++ using multi-threading, custom event types, and a ring buffer for efficient inter-thread communication. This project demonstrates a simplified architecture for a trading system, including market data ingestion, strategy execution, and order management.

## Features
- **Multi-threaded Design:** Separate threads for feed handling, strategy, and order execution.
- **Custom Event Types:** `MarketDataEvent` and `OrderEvent` for passing data between components.
- **Ring Buffer:** Lock-free buffer for fast, thread-safe communication.
- **Logger:** Flexible logging to console and file, with thread safety.
- **Atomic Counters:** For tracking events, orders, and profit metrics.
- **Graceful Shutdown:** Uses C++20 `std::jthread` and `std::stop_token` for clean thread termination.

## Project Structure
```
├── src/
│   ├── main.cpp            # Entry point, orchestrates threads and summary
│   ├── events.hpp          # Event type definitions
│   ├── ring_buffer.hpp     # Lock-free ring buffer implementation
│   ├── logger.cpp/hpp      # Thread-safe logging utility
│   ├── feed_handler.cpp    # Market data ingestion logic
│   ├── strategy.cpp        # Trading strategy logic
│   └── order_executor.cpp  # Order execution logic
└── CMakeLists.txt          # Build configuration
```

## How It Works
1. **Market Data Feed:**
   - `run_feed_handler` generates market events and pushes them to the ring buffer.
2. **Strategy Thread:**
   - `run_strategy` consumes market events, applies trading logic, and generates orders.
3. **Order Executor:**
   - `run_order_executor` processes orders and simulates execution.
4. **Logger:**
   - Logs all major actions and summary statistics at the end of the run.

## Usage
### Build
```bash
mkdir -p build
cd build
cmake ..
make
```
### Run
```bash
./out
```

### Output
The program runs for 3 seconds, then prints a summary:
- Market Events generated
- Orders created and executed
- Total spent and earned
- Position and average cost
- Net profit

## Customization
- **Buffer Size:** Change the size in `main.cpp` for different throughput.
- **Strategy Logic:** Modify `strategy.cpp` to implement your own trading logic.
- **Logger:** Configure console/file logging in `logger.hpp` and `logger.cpp`.

## Dependencies
- `C++20` (for `std::jthread`, `std::stop_token`, etc.)
- `CMake` (for building)

## Future
- Integrate with real market data sources (currently uses `std::random` for data).
- Implement more advanced strategies.
- Better logging.

