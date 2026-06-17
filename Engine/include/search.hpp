#include "move.hpp"

class GameState;

class Search {
private:
    GameState* m_state;

public:
    Search(GameState* state) : m_state{state} { };

    Move bestMove(int depth);
};