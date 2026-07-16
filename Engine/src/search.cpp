#include <array>
#include <chrono>
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
        return state->inCheck() ? -INT_MAX : 0;

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
    
    if constexpr (type == SearchType::depth) 
        alphaBeta(state, moveLine, -INT_MAX, INT_MAX, depth);
    else if constexpr (type == SearchType::time) {
        int base = state->getTime(), increment = state->getInc();
        int moveTime = GameState::getMoveTime(base, increment);

        auto start = std::chrono::high_resolution_clock::now();
        int durationMS, currPly = 1;
        do {
            int score = alphaBeta(state, moveLine, -INT_MAX, INT_MAX, currPly++);
            std::cout << "info depth " << currPly << " score cp " << score << " pv";
            for(Move move : moveLine)
                std::cout << " " << Board::getMoveString(move);
            std::cout << std::endl;
            auto current = std::chrono::high_resolution_clock::now();
            durationMS = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(current - start).count());
        } while(durationMS < moveTime);
    }

    std::cout << "bestmove " << Board::getMoveString(moveLine[0]);
    if(moveLine.size() > 1)
        std::cout << " ponder " << Board::getMoveString(moveLine[1]) << std::endl;
    else 
        std::cout << std::endl; 
}