#include <chrono>
#include "stdint.h"
#include "GameState.h"

constexpr int MAX_DEPTH = 5; 

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

int main() {
    GameState game{};

    std::cout << "Enter position FEN string (0 for initial): ";
    std::string FEN;
    std::getline(std::cin, FEN);
    if(FEN != "0")
        game.loadPosition(FEN);

    for(int depth = 0; depth <= MAX_DEPTH; ++depth) {
        uint64_t epCaptures = 0, captures = 0, castles = 0;
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft(&game, epCaptures, captures, castles, depth);
        auto end = std::chrono::high_resolution_clock::now();
        double dur = std::chrono::duration<double>(end - start).count();
    
        std::cout << "Depth " << depth << ":\n";
        std::cout << "Nodes: " << nodes << '\n';
        std::cout << "Castles: " << castles << '\n';
        std::cout << "Captures: " << captures << '\n';
        std::cout << "En Passant Captures: " << epCaptures << '\n';
        std::cout << "Time: " << dur << "s\n";
        std::cout << "Nodes per second: " << static_cast<int>(nodes / dur) << "\n\n";
    }

    return 0;
}