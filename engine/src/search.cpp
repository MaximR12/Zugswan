#include <array>
#include <chrono>
#include <atomic>
#include <thread>
#include <limits>
#include "transposetable.hpp"
#include "search.hpp"
#include "gamestate.hpp"

std::atomic<bool> stopRequested = false;

int16_t evaluate(GameState* state) {
    return state->getTurn() == Board::white ? Board::materialBalance(state->getBoard()) : -Board::materialBalance(state->getBoard());
}

int16_t alphaBeta(GameState* state, SearchMetrics& metrics, FixedVector<Move, MAX_SEARCH_DEPTH>& prevMoveLine, int16_t alpha, int16_t beta, int depth) {
    if(stopRequested.load(std::memory_order_relaxed))
        return 0;

    ++metrics.nodes;

    if(depth == 0) 
        return evaluate(state);

    FixedVector<Move, MAX_LEGAL_MOVES> moveList;
    state->getLegalMoves(moveList);

    if(moveList.size() == 0) //check mate / stalemate
        return state->inCheck() ? -std::numeric_limits<int16_t>::max() : 0;

    if(state->getHalfMoveClock() >= 100) //check 50 move rule / three move repitition
        return 0;

    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;

    uint64_t zobrist = state->getZobrist();
    TransposeEntry* tEntry = Tables::TTable.probe(zobrist);
    if(tEntry) {
        ++metrics.ttHits;

        if(tEntry->depth >= depth) { //check if early cut off is possible
            switch(tEntry->type) {
                case(NodeType::exact):
                    prevMoveLine[0] = tEntry->best;
                    prevMoveLine.push_vec(moveLine, 1);
                    return tEntry->score;
                    
                case(NodeType::upper):
                    if(tEntry->score <= alpha) {
                        prevMoveLine[0] = tEntry->best;
                        prevMoveLine.push_vec(moveLine, 1);
                        return tEntry->score;
                    }
                    break;
                
                case(NodeType::lower):
                    if(tEntry->score >= beta) {
                        prevMoveLine[0] = tEntry->best;
                        prevMoveLine.push_vec(moveLine, 1);
                        return tEntry->score;
                    }
                    break;
            }
        }

        moveList.reorder(tEntry->best); //if no early cut off is possible, reorder the previous found best move to front of move list
    }
    ++metrics.ttTotal;

    NodeType type = NodeType::upper;
    int bestScore = -std::numeric_limits<int16_t>::max();
    Move bestMove = moveList[0];

    for(Move move : moveList) {
        state->makeMove(move);
        int16_t score = -alphaBeta(state, metrics, moveLine, -beta, -alpha, depth-1);
        state->unmakeMove(move);

        if(score > bestScore) {
            bestScore = score, bestMove = move;
            prevMoveLine[0] = move;
            prevMoveLine.push_vec(moveLine, 1);

            if(bestScore > alpha) {
                alpha = bestScore;
                type = NodeType::exact;
            }
        }

        if(score >= beta) {
            Tables::TTable.insert(zobrist, NodeType::lower, move, depth, score);
            prevMoveLine[0] = move;
            prevMoveLine.push_vec(moveLine, 1);
            return bestScore;
        }
    }

    if(!stopRequested.load(std::memory_order_relaxed))
        Tables::TTable.insert(zobrist, type, bestMove, depth, bestScore);
    
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

    int64_t score = 0;
    for(int ply = 1; ply < MAX_SEARCH_DEPTH; ++ply) {
        if(score == -std::numeric_limits<int16_t>::max() || score == std::numeric_limits<int16_t>::max()) //exit early if forced mate
            break;

        SearchMetrics metrics;
        FixedVector<Move, MAX_SEARCH_DEPTH> currMoveLine;
        score = alphaBeta(state, metrics, currMoveLine, -std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::max(), ply);
        
        if(stopRequested)
            break;
        
        moveLine = currMoveLine;
        std::cout << "info depth " << ply << " score cp " << score << " nodes " << metrics.nodes << " pv";
        for(Move move : moveLine)
            std::cout << " " << Board::getMoveString(move);
        std::cout << std::endl;
    } 
}

template<SearchType type>
void Search::Search(GameState* state, int depth) {
    Board::PieceColor turn = state->getTurn();
    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;
    
    if constexpr (type == SearchType::depth) {
        stopRequested = false;
        SearchMetrics metrics;
        alphaBeta(state, metrics, moveLine, -std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::max(), depth);
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