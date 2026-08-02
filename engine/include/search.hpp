#include "move.hpp"
#include <limits>

class GameState;

constexpr int MAX_SEARCH_DEPTH = 32;

constexpr int16_t VALUE_MATE = 30'000;
constexpr int16_t VALUE_INFINITE = 30'001;
constexpr int16_t VALUE_DRAW = 0;

constexpr int16_t ASPIRATION_WIDTH = 200;

enum class SearchType {
    time, depth, nodes, movetime, infinite
};

struct SearchMetrics {
    int nodes = 0;
    int ttHits = 0;
    int ttTotal = 0;
};

namespace Search {
    template<SearchType type>
    void Search(GameState* state, int depth=0); 

    template void Search<SearchType::depth>(GameState*, int);
    template void Search<SearchType::time>(GameState*, int);
};