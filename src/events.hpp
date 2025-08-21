#pragma once
#include <cstdint>

// Represents a market data event, such as a trade or quote update.
struct MarketDataEvent {
    uint64_t timestamp; // Event timestamp in nanoseconds since epoch
    double price;       // Price of the instrument
    int quantity;       // Quantity traded or quoted
    char side;          // 'B' for buy, 'S' for sell
};

// Represents an order event, such as a new order or cancellation.
struct OrderEvent {
    uint64_t timestamp; 
    double price;       
    int quantity;       
    char side;          
};

