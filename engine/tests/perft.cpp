#include "perft.hpp"
#include "fixedvector.hpp"
#include <chrono>
#include <map>

uint64_t perft(GameState* game, int depth) {
    FixedVector<Move, MAX_LEGAL_MOVES> moveList;
    game->getLegalMoves(moveList);
    uint64_t nodes = 0;

    if(depth == 1) 
        return moveList.size();

    for(size_t i = 0; i < moveList.size(); ++i) {
        game->makeMove(moveList[i]);
        nodes += perft(game, depth-1);
        game->unmakeMove(moveList[i]);
    }

    return nodes;
}

template<Perft::Mode mode>
void Perft::run(GameState* game, int depth) {
    if constexpr (mode == divide) {
        FixedVector<Move, MAX_LEGAL_MOVES> moveList;
        game->getLegalMoves(moveList);
        uint64_t total = 0;

        std::map<std::string, uint64_t> movePaths;
        for(size_t i = 0; i < moveList.size(); ++i) {
            game->makeMove(moveList[i]);
            uint64_t nodes = depth == 1 ? 1 : perft(game, depth-1);
            game->unmakeMove(moveList[i]);

            total += nodes;
            movePaths[Board::getMoveString(moveList[i])] = nodes;
        }

        for(auto &[path, nodes] : movePaths) 
            std::cout << path << ": " << nodes << '\n';

        std::cout << "\nNodes searched: " << total << '\n';
    }

    if constexpr (mode == bench) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft(game, depth);
        auto end = std::chrono::high_resolution_clock::now();
        double dur = std::chrono::duration<double>(end - start).count();

        std::cout << "Time: " << dur << "s\n";
        std::cout << "Nodes searched: " << nodes << '\n'; 
        std::cout << "Nodes per second: " << static_cast<int>(nodes / dur) << '\n';
    }
}