#include "Engine.h"
#include <chrono>
#include <random>

const Move Engine::getTopMove(const Turn turn) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Move> legalMoves = m_moveGen->getLegalMoves(turn);
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Move gen time: " << dur << '\n';

    int ind = std::rand() % legalMoves.size();
    return legalMoves[ind];
}