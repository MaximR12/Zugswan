#include <array>
#include <chrono>
#include <atomic>
#include <thread>
#include "transposetable.hpp"
#include "search.hpp"
#include "gamestate.hpp"

std::atomic<bool> stopRequested = false;

int evaluate(GameState* state) {
    return state->getTurn() == Board::white ? Board::materialBalance(state->getBoard()) : -Board::materialBalance(state->getBoard());
}

int alphaBeta(GameState* state, SearchMetrics& metrics, FixedVector<Move, MAX_SEARCH_DEPTH>& prevMoveLine, int alpha, int beta, int depth) {
    if(stopRequested.load(std::memory_order_relaxed))
        return 0;

    ++metrics.nodes;

    if(depth == 0) 
        return evaluate(state);

    FixedVector<Move, MAX_LEGAL_MOVES> moveList;
    state->getLegalMoves(moveList);

    if(moveList.size() == 0) 
        return state->inCheck() ? -INT_MAX : 0;

    uint64_t zobrist = state->getZobrist();
    TransposeEntry* tEntry = Tables::TTable.probe(zobrist);
    if(tEntry)
        moveList.reorder(tEntry->best), ++metrics.ttHits;
    else
        ++metrics.ttMisses;

    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;
    for(Move move : moveList) {
        state->makeMove(move);
        int score = -alphaBeta(state, metrics, moveLine, -beta, -alpha, depth-1);
        state->unmakeMove(move);

        if(score >= beta) {
            Tables::TTable.insert(zobrist, NodeType::pv, move, depth, score);
            prevMoveLine[0] = move;
            prevMoveLine.push_vec(moveLine, 1);
            return beta;
        }
        
        if(score > alpha) {
            Tables::TTable.insert(zobrist, NodeType::pv, move, depth, score);
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
        SearchMetrics metrics;
        FixedVector<Move, MAX_SEARCH_DEPTH> currMoveLine;
        int score = alphaBeta(state, metrics, currMoveLine, -INT_MAX, INT_MAX, currPly++);
        
        if(!stopRequested)
            moveLine = currMoveLine;

        std::cout << "info depth " << currPly-1 << " score cp " << score << " nodes " << metrics.nodes << " pv";
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
        SearchMetrics metrics;
        alphaBeta(state, metrics, moveLine, -INT_MAX, INT_MAX, depth);
        std::cout << "nodes " << metrics.nodes << std::endl;
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