#include "perft.hpp"
#include <chrono>
#include <map>

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

void Perft::run(GameState* game, int depth) {
    std::array<Move, MAX_LEGAL_MOVES> moveBuf;
    size_t numMoves = game->getLegalMoves(moveBuf);
    uint64_t total = 0;

    std::map<std::string, uint64_t> movePaths;
    for(size_t i = 0; i < numMoves; ++i) {
        game->makeMove(moveBuf[i]);
        uint64_t nodes = depth == 1 ? 1 : perft(game, depth-1);
        game->unMakeMove();

        total += nodes;
        movePaths[Board::getMoveString(moveBuf[i])] = nodes;
    }

    for(auto &[path, nodes] : movePaths) 
        std::cout << path << ": " << nodes << '\n';

    std::cout << "\nNodes searched: " << total << '\n';
}