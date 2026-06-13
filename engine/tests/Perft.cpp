#include "Perft.h"
#include <chrono>
#include <map>

uint64_t perft(GameState* game, uint64_t& epCaptures, uint64_t& captures, uint64_t& castles, int depth) {
    std::array<Move, MAX_LEGAL_MOVES> moveBuf;
    uint64_t nodes = 0;

    if(depth == 0) 
        return 1ULL;

    size_t numMoves = game->getLegalMoves(moveBuf);
    for(size_t i = 0; i < numMoves; ++i) {
        if(depth == 1) {
            uint16_t flag = moveBuf[i].getFlag();
            if(flag == QUEEN_CASTLE || flag == KING_CASTLE) ++castles;
            if(flag == EP_CAPTURE) ++epCaptures;
            if(moveBuf[i].isCapture()) ++captures;
        }
        game->makeMove(moveBuf[i]);
        nodes += perft(game, epCaptures, captures, castles, depth-1);
        game->unMakeMove();
    }

    return nodes;
}

void runPerft(GameState* game, int depth) {
    for(int i = 0; i <= depth; ++i) {
        uint64_t epCaptures = 0, captures = 0, castles = 0;
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft(game, epCaptures, captures, castles, i);
        auto end = std::chrono::high_resolution_clock::now();
        double dur = std::chrono::duration<double>(end - start).count();
    
        std::cout << "Depth " << i << ":\n";
        std::cout << "Nodes: " << nodes << '\n';
        std::cout << "Castles: " << castles << '\n';
        std::cout << "Captures: " << captures << '\n';
        std::cout << "En Passant Captures: " << epCaptures << '\n';
        std::cout << "Time: " << dur << "s\n";
        std::cout << "Nodes per second: " << static_cast<int>(nodes / dur) << "\n\n";
    }
}

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

void runPerftBulk(GameState* game, int depth) {
    for(int i = 1; i <= depth; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perftBulk(game, i);
        auto end = std::chrono::high_resolution_clock::now();
        double dur = std::chrono::duration<double>(end - start).count();

        std::cout << "Depth " << i << ":\n";
        std::cout << "Nodes: " << nodes << '\n';
        std::cout << "Time: " << dur << "s\n";
        std::cout << "Nodes per second: " << static_cast<int>(nodes / dur) << "\n\n";
    }
}

void runDivide(GameState* game, int depth) {
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