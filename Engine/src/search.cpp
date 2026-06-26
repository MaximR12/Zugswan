#include <array>
#include "search.hpp"
#include "gamestate.hpp"

int evaluate(GameState* state) {
    return state->getTurn() == Board::white ? Board::materialBalance(state->getBoard()) : -Board::materialBalance(state->getBoard());
}

int Search::alphaBetaMax(GameState* state, int& alpha, int& beta, int depthLeft) {
    if(depthLeft == 0)
        return evaluate(state);

    std::array<Move, MAX_LEGAL_MOVES> legalMoves;
    uint16_t numLegalMoves = state->getLegalMoves(legalMoves);

    int bestValue = INT_MIN;
    for(int i = 0; i < numLegalMoves; ++i) {
        state->makeMove(legalMoves[i]);
        int score = alphaBetaMin(state, alpha, beta, depthLeft-1);
        state->unMakeMove();

        if(score > bestValue) {
            bestValue = score;
            if(score > alpha)
                alpha = score;
        }

        if(score >= beta)
            return score;
    }

    return bestValue;
}

int Search::alphaBetaMin(GameState* state, int& alpha, int& beta, int depthLeft) {
    if(depthLeft == 0)
        return -evaluate(state);

    std::array<Move, MAX_LEGAL_MOVES> legalMoves;
    uint16_t numLegalMoves = state->getLegalMoves(legalMoves);

    int bestValue = INT_MAX;
    for(int i = 0; i < numLegalMoves; ++i) {
        state->makeMove(legalMoves[i]);
        int score = alphaBetaMax(state, alpha, beta, depthLeft-1);
        state->unMakeMove();

        if(score < bestValue) {
            bestValue = score;
            if(score < beta)
                beta = score;
        }

        if(score <= alpha)
            return score;
    }

    return bestValue;
}

Move Search::bestMove(GameState* state, int depth) {
    std::array<Move, MAX_LEGAL_MOVES> legalMoves;
    uint16_t numLegalMoves = state->getLegalMoves(legalMoves);
    assert(numLegalMoves > 0);

    int bestValue = INT_MIN, bestMoveInd = 0;
    for(int i = 1; i < numLegalMoves; ++i) {
        int alpha = INT_MIN, beta = INT_MAX;
        state->makeMove(legalMoves[i]);
        int score = alphaBetaMin(state, alpha, beta, depth-1);
        state->unMakeMove();

        if(score > bestValue) {
            bestValue = score;
            bestMoveInd = i;
        }
    }

    return legalMoves[bestMoveInd];
}