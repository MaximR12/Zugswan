#include "Engine.h"
#include <chrono>
#include <random>

const Move Engine::getTopMove(const Board::PieceColor turn) {
    static auto s = std::chrono::high_resolution_clock::now();
    static auto total = s - s;
    static int count = 0;

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Move> legalMoves = m_moveGen->getLegalMoves(turn);
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    total += dur, count++;

    std::cout << "Move gen time: " << dur << " Current Average: " << total / count << '\n';

    int ind = std::rand() % legalMoves.size();
    return legalMoves[ind];
}