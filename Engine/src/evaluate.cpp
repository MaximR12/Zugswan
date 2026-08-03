#include "evaluate.hpp"

int16_t getSideMultiple(Board::PieceColor turn) { //return -1 for black and 1 for white
    return -turn + Board::getOppositeColor(turn);
}

int16_t Eval::evaluate(GameState* state) {
    constexpr float materialWeight = 1.0f;
    constexpr float pstWeight = 1.0f;

    int16_t sideMultiple = getSideMultiple(state->getTurn());
    int16_t materialBalance = Board::materialBalance(state->getBoard());
    int16_t pstScore = state->getPst(Board::white) - state->getPst(Board::black);
    
    int16_t eval = static_cast<int16_t>(materialBalance * materialWeight + pstScore * pstWeight);
    return eval * sideMultiple;
}