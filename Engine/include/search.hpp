#include "move.hpp"

class GameState;

class Search {
private:    
    int alphaBetaMax(GameState* state, int& alpha, int& beta, int depthLeft);
    int alphaBetaMin(GameState* state, int& alpha, int& beta, int depthLeft);

public:
    Search() { };

    Move bestMove(GameState* state, int depth);
};