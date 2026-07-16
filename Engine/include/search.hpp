#include "move.hpp"

class GameState;

constexpr int MAX_SEARCH_DEPTH = 32;

enum class SearchType {
    time, depth, nodes, movetime, infinite
};

namespace Search {
    template<SearchType type>
    void Search(GameState* state, int depth=0); 

    template void Search<SearchType::depth>(GameState*, int);
    template void Search<SearchType::time>(GameState*, int);
};