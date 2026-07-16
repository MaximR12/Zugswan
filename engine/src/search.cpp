#include <array>
#include "search.hpp"
#include "gamestate.hpp"

int evaluate(GameState* state) {
    return state->getTurn() == Board::white ? Board::materialBalance(state->getBoard()) : -Board::materialBalance(state->getBoard());
}

int alphaBeta(GameState* state, FixedVector<Move, MAX_SEARCH_DEPTH>& prevMoveLine, int alpha, int beta, int depth) {
    if(depth == 0) 
        return evaluate(state);

    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;
    FixedVector<Move, MAX_LEGAL_MOVES> moveList;
    state->getLegalMoves(moveList);

    if(moveList.size() == 0)
        return state->inCheck() ? -1000 : 0;

    for(Move move : moveList) {
        state->makeMove(move);
        int score = -alphaBeta(state, moveLine, -beta, -alpha, depth-1);
        state->unmakeMove(move);

        if(score >= beta)
            return beta;

        if(score > alpha) {
            alpha = score;
            prevMoveLine[0] = move;
            prevMoveLine.push_vec(moveLine, 1);
        }
    }

    return alpha;
}

template<SearchType type>
void Search::Search(GameState* state, int depth) {
    Board::PieceColor turn = state->getTurn();
    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;
    
    int score;
    if constexpr (type == SearchType::depth) 
        score = alphaBeta(state, moveLine, -1000, 1000, depth);

    std::cout << "Value: " << score << '\n';
    for(Move move : moveLine)
        std::cout << Board::getMoveString(move) << '\n';
}