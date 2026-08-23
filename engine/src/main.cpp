#include "uci.hpp"

int main() {
    Board::printBitBoard(INDEX_MASK);
    Tables::init();

    GameState game{};
    UCI uci{&game};

    uci.run();

    return 0;
}