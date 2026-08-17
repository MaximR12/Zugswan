#include "move.hpp"
#include <limits>

class GameState;

constexpr int MAX_SEARCH_DEPTH = 32;

constexpr int16_t VALUE_MATE = 30'000;
constexpr int16_t VALUE_DRAW = 0;

constexpr int16_t ASPIRATION_WIDTH = 75;

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
    SearchMetrics Search(GameState* state, int depth=0, int movetime=0);

    void requestStop();

    template SearchMetrics Search<SearchType::depth>(GameState*, int, int);
    template SearchMetrics Search<SearchType::time>(GameState*, int, int);
    template SearchMetrics Search<SearchType::movetime>(GameState*, int, int);
};