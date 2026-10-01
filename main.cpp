#include "Orderbook.h"
#include <memory>

int main() {
    Orderbook Orderbook{};

    OrderPointer order1 = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
    
}