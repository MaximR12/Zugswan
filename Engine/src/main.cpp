#include "uci.hpp"

int main() {
    GameState game{};
    Engine engine{&game};
    UCI uci{&game, &engine};

    uci.run();

    return 0;
}