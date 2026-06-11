#include <chrono>
#include <map>
#include <algorithm>
#include "stdint.h"
#include "GameState.h"

constexpr int MAX_DEPTH = 7;

uint64_t perftBulk(GameState* game, int depth) {
    std::array<Move, MAX_LEGAL_MOVES> moveBuf;
    size_t numMoves = game->getLegalMoves(moveBuf);
    uint64_t nodes = 0;

    if(depth == 1) 
        return numMoves;

    for(size_t i = 0; i < numMoves; ++i) {
        game->makeMove(moveBuf[i]);
        nodes += perftBulk(game, depth-1);
        game->unMakeMove();
    }

    return nodes;
}

void divide(GameState* game, int depth) {
    std::array<Move, MAX_LEGAL_MOVES> moveBuf;
    size_t numMoves = game->getLegalMoves(moveBuf);
    uint64_t total = 0;

    std::map<std::string, uint64_t> movePaths;
    for(size_t i = 0; i < numMoves; ++i) {
        game->makeMove(moveBuf[i]);
        uint64_t nodes = depth == 1 ? 1 : perftBulk(game, depth-1);
        game->unMakeMove();

        total += nodes;
        movePaths[Board::getMoveString(moveBuf[i])] = nodes;
    }

    for(auto &[path, nodes] : movePaths) 
        std::cout << path << ": " << nodes << '\n';

    std::cout << "\nNodes searched: " << total << '\n';
}

int main() {
    GameState game{};

    std::cout << "Enter position FEN string (0 for initial): ";
    std::string FEN;
    std::getline(std::cin, FEN);
    if(FEN != "0")
        game.loadPosition(FEN);

    for(int depth = 1; depth <= MAX_DEPTH; ++depth) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perftBulk(&game, depth);
        auto end = std::chrono::high_resolution_clock::now();
        double dur = std::chrono::duration<double>(end - start).count();
    
        std::cout << "Depth " << depth << ":\n";
        std::cout << "Nodes: " << nodes << '\n';
        std::cout << "Time: " << dur << "s\n";
        std::cout << "Nodes per second: " << static_cast<int>(nodes / dur) << "\n\n";
    }
}