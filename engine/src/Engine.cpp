#include "Engine.h"
#include <random>

const Move Engine::getTopMove(const Turn turn) {
    std::vector<Move> legalMoves = m_moveGen->getLegalMoves(turn);
    int ind = std::rand() % legalMoves.size();
    return legalMoves[ind];
}