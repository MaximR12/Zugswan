#pragma once

#include "gamestate.hpp"
#include "stdint.h"

namespace Eval {

    int16_t evaluate(GameState* state, int16_t alpha, int16_t beta, bool& isLazy);

}