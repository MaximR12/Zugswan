#include "evaluate.hpp"

int16_t getSideMultiple(Board::PieceColor turn) { //return -1 for black and 1 for white
    return -turn + Board::getOppositeColor(turn);
}

int16_t Eval::evaluate(GameState* state) {
    constexpr float materialWeight = 1.0f;
    constexpr float pstWeight = 1.0f;

    int16_t sideMultiple = getSideMultiple(state->getTurn());
    int16_t materialBalance = Board::materialBalance(state->getBoard());

    int16_t pstScoreMid = state->getPst(Board::middle, Board::white) - state->getPst(Board::middle, Board::black);
    int16_t pstScoreEnd = state->getPst(Board::end, Board::white) - state->getPst(Board::end, Board::black);
    int16_t phase = state->getPhase();
 
    int16_t evalMid = static_cast<int16_t>(materialBalance * materialWeight + pstScoreMid * pstWeight);
    int16_t evalEnd = static_cast<int16_t>(materialBalance * materialWeight + pstScoreEnd * pstWeight);
    int16_t eval = ((evalMid * (256 - phase)) + (evalEnd * phase)) / 256;
    
    return eval * sideMultiple;
}