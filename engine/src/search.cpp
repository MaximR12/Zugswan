#include <array>
#include <chrono>
#include <atomic>
#include <thread>
#include "search.hpp"
#include "gamestate.hpp"

std::atomic<bool> stopRequested = false;

int evaluate(GameState* state) {
    return state->getTurn() == Board::white ? Board::materialBalance(state->getBoard()) : -Board::materialBalance(state->getBoard());
}

int alphaBeta(GameState* state, FixedVector<Move, MAX_SEARCH_DEPTH>& prevMoveLine, int alpha, int beta, int depth) {
    if(stopRequested.load(std::memory_order_relaxed))
        return 0;

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

        if(score >= beta) {
            prevMoveLine[0] = move;
            prevMoveLine.push_vec(moveLine, 1);
            return beta;
        }
        
        if(score > alpha) {
            alpha = score;
            prevMoveLine[0] = move;
            prevMoveLine.push_vec(moveLine, 1);
        }
    }

    return alpha;
}

void iterativeDeepening(GameState* state, FixedVector<Move, MAX_SEARCH_DEPTH>& moveLine) {
    int base = state->getTime(), increment = state->getInc();
    int moveTime = GameState::getMoveTime(base, increment);
    std::thread timer([moveTime]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(moveTime));
        stopRequested.store(true, std::memory_order_relaxed);
    });
    timer.detach();

    int currPly = 1;
    do {
        FixedVector<Move, MAX_SEARCH_DEPTH> currMoveLine;
        int score = alphaBeta(state, currMoveLine, -INT_MAX, INT_MAX, currPly++);
        
        if(!stopRequested)
            moveLine = currMoveLine;

        std::cout << "info depth " << currPly-1 << " score cp " << score << " pv";
        for(Move move : moveLine)
            std::cout << " " << Board::getMoveString(move);
        std::cout << std::endl;
    } while(!stopRequested);
}

template<SearchType type>
void Search::Search(GameState* state, int depth) {
    Board::PieceColor turn = state->getTurn();
    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;
    
    if constexpr (type == SearchType::depth) {
        stopRequested = false;
        alphaBeta(state, moveLine, -INT_MAX, INT_MAX, depth);
    } else if constexpr (type == SearchType::time) {
        stopRequested = false;
        iterativeDeepening(state, moveLine);
    }

    std::cout << "bestmove " << Board::getMoveString(moveLine[0]);
    if(moveLine.size() > 1)
        std::cout << " ponder " << Board::getMoveString(moveLine[1]) << std::endl;
    else 
        std::cout << std::endl; 
}