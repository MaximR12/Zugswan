#include "uci.hpp"

int main() { 
    Tables::init();

    GameState game{};
    UCI uci{&game};

    uci.run();

    return 0;
}