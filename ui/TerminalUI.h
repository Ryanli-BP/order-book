#pragma once

#include <string>

#include "Orderbook.h"

class TerminalUI {
public:
    explicit TerminalUI(Orderbook& orderbook)
        : orderbook_ { orderbook }
    {
    }

    void Run();

private:
    void Draw() const;
    void HandleCommand(const std::string& line);

    Orderbook& orderbook_;
    OrderId orderId_ { 1 };
    std::string status_ { "welcome! type 'help' to see the available commands" };
};
