#include "TerminalUI.h"

#include <format>
#include <iostream>
#include <memory>
#include <ranges>
#include <sstream>

namespace {
    const std::string helpText{
        "commands:\n"
        "  buy|sell <price> <qty> <gtc|gfd|fak|fok>   place a limit order\n"
        "  market buy|sell <qty>                      place a market order\n"
        "  cancel <id>                                cancel a resting order\n"
        "  modify <id> buy|sell <price> <qty>         replace a resting order\n"
        "  eod                                        end of day: cancel all gfd orders\n"
        "  help                                       show this list\n"
        "  Ctrl+D                                     quit"
    };

    void DisplayOrderbook (const OrderbookLevelInfos& orderbookLevelInfos) {
        std::string askLine = std::format("{:^20}", "ASKS");
        std::cout << askLine << std::endl;

        const auto& asks = orderbookLevelInfos.GetAsks();
        const auto& bids = orderbookLevelInfos.GetBids();

        std::cout << "\033[31m";
        for (const auto& level : asks | std::views::reverse) {
            std::cout << std::format("{:>8}{:>8}", level.price_, level.quantity_) << std::endl;
        }
        std::cout << "\033[0m";

        std::string spreadText = " spread ";
        if (!asks.empty() && !bids.empty())
            spreadText = std::format(" spread {} ", asks.front().price_ - bids.front().price_);
        std::cout << std::format("{:-^20}", spreadText) << std::endl;

        std::cout << "\033[32m";
        for (const auto& bidLevelInfo : bids) {
            std::cout << std::format("{:>8}{:>8}", bidLevelInfo.price_, bidLevelInfo.quantity_) << std::endl;
        }
        std::cout << "\033[0m";
        std::cout << std::format("{:^20}", "BIDS");
    }

    std::istream& operator>>(std::istream& in, Side& side) {
        std::string sideWord;
        in >> sideWord;
        if (sideWord == "buy") {
            side = Side::Buy;
        }
        else if (sideWord == "sell") {
            side = Side::Sell;
        }
        else {
            in.setstate(std::ios::failbit);
        }

        return in;
    }

    std::istream& operator>>(std::istream& in, OrderType& type) {
        std::string typeWord;
        in >> typeWord;
        if (typeWord == "fak") {
            type = OrderType::FillAndKill;
        }
        else if (typeWord == "gtc") {
            type = OrderType::GoodTillCancel;
        }
        else if (typeWord == "fok") {
            type = OrderType::FillOrKill;
        }
        else if (typeWord == "gfd") {
            type = OrderType::GoodForDay;
        }
        else {
            in.setstate(std::ios::failbit);
        }

        return in;
    }
}

void TerminalUI::Run() {
    while (true) {
        Draw();

        std::string line;
        if (!std::getline(std::cin, line))
            break;

        HandleCommand(line);
    }
}

void TerminalUI::Draw() const {
    std::cout << "\033[2J\033[H";
    DisplayOrderbook(orderbook_.GetOrderInfos());

    std::cout << "\n" << status_ << "\n\n> ";   // show last result, then the prompt
}

void TerminalUI::HandleCommand(const std::string& line) {
    std::istringstream in(line);
    std::string command;
    in >> command;

    if (command == "buy" || command == "sell") {
        Side side = (command == "buy") ? Side::Buy : Side::Sell;
        Price price;
        Quantity quantity;

        OrderType type{};
        if (!(in >> price >> quantity >> type)) {
            status_ = "usage : [buy|sell] <price> <quantity> <gtc|gfd|fak|fok>";
            return;
        }

        const auto& trades = orderbook_.AddOrder(type, orderId_, side, price, quantity);
        status_ = std::format("#{} {} {} @ {}, {} trade(s)", orderId_, command, quantity, price, trades.size());
        ++orderId_;
    }
    else if (command == "market") {
        Side side;
        Quantity quantity;
        if (!(in >> side >> quantity)) {
            status_ = "usage : market [buy|sell] <quantity>";
            return;
        }

        const auto& trades = orderbook_.AddOrder(OrderType::Market, orderId_, side, InvalidPrice, quantity);
        status_ = std::format("#{} market {} {}, {} trade(s)", orderId_, side == Side::Buy ? "buy" : "sell", quantity, trades.size());
        ++orderId_;
    }
    else if (command == "cancel") {
        OrderId id;
        if (!(in >> id)) {
            status_ = "usage : cancel <id>";
            return;
        }

        if (!orderbook_.HasOrder(id)) {
            status_ = std::format("no resting order #{}", id);
            return;
        }

        orderbook_.CancelOrder(id);
        status_ = std::format("cancelled #{}", id);
    }
    else if (command == "modify") {
        OrderId id;
        Side side;
        Price price;
        Quantity quantity;
        if (!(in >> id >> side >> price >> quantity)) {
            status_ = "usage : modify <id> [buy|sell] <price> <quantity>";
            return;
        }

        if (!orderbook_.HasOrder(id)) {
            status_ = std::format("no resting order #{}", id);
            return;
        }

        auto trades = orderbook_.ModifyOrder(OrderModify{ id, side, price, quantity });
        status_ = std::format("modify #{} -> {} {} @ {}, {} trade(s)", id, side == Side::Buy ? "buy" : "sell", quantity, price, trades.size());
    }
    else if (command == "eod") {
        auto sizeBefore = orderbook_.Size();
        orderbook_.EndOfDay();
        status_ = std::format("end of day: cancelled {} gfd order(s)", sizeBefore - orderbook_.Size());
    }
    else if (command == "help") {
        status_ = helpText;
    }
    else if (command.empty()) {
        status_ = "";
    }
    else {
        status_ = std::format("unknown command '{}' (type 'help' for the list)", command);
    }
}
