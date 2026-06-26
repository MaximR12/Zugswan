#pragma once

#include "stdint.h"
#include "gamestate.hpp"

namespace Perft {

enum Mode {
    normal, bench
};

template<Mode mode>
void run(GameState* game, int depth);

template void run<normal>(GameState*, int);
template void run<bench>(GameState*, int);

}
