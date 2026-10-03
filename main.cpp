#include "Orderbook.h"
#include "TerminalUI.h"

int main() {
    Orderbook orderbook{};
    TerminalUI ui{ orderbook };
    ui.Run();
}
