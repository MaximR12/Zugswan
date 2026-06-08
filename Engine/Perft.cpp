#include <chrono>
#include "stdint.h"
#include "GameState.h"

constexpr int MAX_DEPTH = 10; 

uint64_t perft(GameState* game, int depth) {
    std::array<Move, MAX_LEGAL_MOVES> moveBuf;
    size_t numMoves = game->getLegalMoves(moveBuf);
    uint64_t nodes = 0;

    if(depth == 1) 
        return numMoves;

    for(size_t i = 0; i < numMoves; ++i) {
        game->makeMove(moveBuf[i]);
        nodes += perft(game, depth-1);
        game->unMakeMove();
    }

    return nodes;
}

int main() {
    GameState game{};
    for(int depth = 1; depth < MAX_DEPTH; ++depth) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft(&game, depth);
        auto end = std::chrono::high_resolution_clock::now();
        double dur = std::chrono::duration<double>(end - start).count();
    
        std::cout << "Depth " << depth << ":\n";
        std::cout << "Nodes: " << nodes << '\n';
        std::cout << "Time: " << dur << "s\n";
        std::cout << "Nodes per second: " << static_cast<int>(nodes / dur) << "\n\n";
    }

    return 0;
}