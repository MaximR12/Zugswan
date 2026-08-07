#include <array>
#include <chrono>
#include <atomic>
#include <thread>
#include <algorithm>
#include "evaluate.hpp"
#include "transposetable.hpp"
#include "search.hpp"
#include "gamestate.hpp"

std::atomic<bool> stopRequested = false;

void startTimer(int timeMS) {
    std::thread timer([timeMS]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeMS));
        stopRequested.store(true, std::memory_order_relaxed);
    });
    timer.detach();
}

void updatePV(FixedVector<Move, MAX_SEARCH_DEPTH>& prevMoveLine, FixedVector<Move, MAX_SEARCH_DEPTH>& moveLine, Move move) {
    prevMoveLine.clear();
    prevMoveLine.push_back(move);
    prevMoveLine.push_vec(moveLine, 1);
}

int16_t quiescenceSearch(GameState* state, SearchMetrics& metrics, int16_t alpha, int16_t beta) {
    if(stopRequested.load(std::memory_order_relaxed))
        return alpha;
    
    ++metrics.nodes;

    constexpr int16_t TEMPO_BONUS = 10;
    int16_t staticEval = Eval::evaluate(state) + TEMPO_BONUS;
    int16_t bestScore = staticEval;
    
    if(bestScore >= beta)
        return bestScore;

    int16_t maxSwing = Board::getPieceValue(Board::queens);
    if(state->promotionPossible())
        maxSwing += Board::getPieceValue(Board::queens) - Board::getPieceValue(Board::pawns);

    if(bestScore < alpha - maxSwing) //early return if max material swing cannot improve alpha
        return bestScore;

    if(bestScore > alpha)
        alpha = bestScore;

    MoveList moveList;
    state->getLegalMoves(moveList);

    if(moveList.size() == 0) //check mate / stalemate
        return state->inCheck() ? -VALUE_MATE : VALUE_DRAW;

    if(state->getHalfMoveClock() >= 100 || state->isRepetition()) //check 50 move rule / three move repitition
        return VALUE_DRAW;

    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;
    Move move;
    
    uint64_t zobrist = state->getZobrist();
    TransposeEntry* tEntry = Tables::TTable.probe(zobrist);
    if(tEntry) {
        ++metrics.ttHits;

        switch(tEntry->type) {
            case(NodeType::exact):
                return tEntry->score;
                
            case(NodeType::lower):
                if(tEntry->score >= beta)
                    return tEntry->score;
                break;

            case(NodeType::upper):
                if(tEntry->score <= alpha)
                    return tEntry->score;
                break;
        }
    }
    ++metrics.ttTotal;
    
    while((move = moveList.pick_move()) != Move::invalid()) {
        if(!move.isCapture())
            continue;

        //delta pruning, skip moves unlikely to raise alpha
        constexpr int16_t delta = 200;
        int16_t captureValue = move.getFlag() != EP_CAPTURE 
            ? Board::getPieceValue(state->getBoard()->getPieceType(move.getTo(), Board::getOppositeColor(state->getTurn()))) : Board::getPieceValue(Board::pawns);
        int16_t futilityValue = delta + captureValue; 
        
        if(captureValue <= alpha - delta) {
            bestScore = std::max(bestScore, futilityValue);
            continue;
        }

        int16_t staticExchangeEval = state->getSEE(move);
        if(staticExchangeEval <= alpha - delta) {
            bestScore = std::max(bestScore, staticExchangeEval);
            continue;
        }

        state->makeMove(move);
        int16_t score = -quiescenceSearch(state, metrics, -beta, -alpha);
        state->unmakeMove(move);

        if(score > bestScore) {
            bestScore = score;
            if(bestScore > alpha) 
                alpha = bestScore;
        }

        if(score >= beta) 
            return bestScore;
    }

    return bestScore;
}

int16_t alphaBeta(GameState* state, SearchMetrics& metrics, FixedVector<Move, MAX_SEARCH_DEPTH>& prevMoveLine, int16_t alpha, int16_t beta, int depth) {
    if(stopRequested.load(std::memory_order_relaxed))
        return alpha;

    if(depth == 0) 
        return quiescenceSearch(state, metrics, alpha, beta);

    ++metrics.nodes;

    MoveList moveList;
    state->getLegalMoves(moveList);

    if(moveList.size() == 0) { //check mate / stalemate
        prevMoveLine.clear();
        return state->inCheck() ? -VALUE_MATE : VALUE_DRAW;
    }

    if(state->getHalfMoveClock() >= 100 || state->isRepetition()) { //check 50 move rule / three move repitition
        if(VALUE_DRAW < beta)
            prevMoveLine.clear();
        return VALUE_DRAW;
    }

    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;
    Move move;
    
    uint64_t zobrist = state->getZobrist();
    TransposeEntry* tEntry = Tables::TTable.probe(zobrist);
    if(tEntry) {
        ++metrics.ttHits;

        if(tEntry->depth >= depth) { //check if early cut off is possible
            switch(tEntry->type) {
                case(NodeType::exact):
                    updatePV(prevMoveLine, moveLine, tEntry->best);
                    return tEntry->score;
                    
                case(NodeType::lower):
                    if(tEntry->score >= beta)
                        return tEntry->score;
                    break;

                case(NodeType::upper):
                    if(tEntry->score <= alpha)
                        return tEntry->score;
                    break;
            }
        }

        move = moveList.pop_move(tEntry->best); //search tt move first
    } else 
        move = moveList.pick_move();
    ++metrics.ttTotal;

    NodeType type = NodeType::upper;
    int16_t bestScore = -VALUE_INFINITE;
    Move bestMove = move;

    while(move != Move::invalid()) { 
        state->makeMove(move);
        int16_t score = -alphaBeta(state, metrics, moveLine, -beta, -alpha, depth-1);
        state->unmakeMove(move);

        if(score > bestScore) {
            bestScore = score, bestMove = move;

            if(bestScore > alpha) {
                alpha = bestScore;
                type = NodeType::exact;
                updatePV(prevMoveLine, moveLine, move);
            }
        }

        if(score >= beta) {
            Tables::TTable.insert(zobrist, NodeType::lower, move, depth, score);
            return bestScore;
        }

        move = moveList.pick_move();
    } 

    if(!stopRequested.load(std::memory_order_relaxed))
        Tables::TTable.insert(zobrist, type, bestMove, depth, bestScore);
    return alpha;
}

template<SearchType type>
void iterativeDeepening(GameState* state, FixedVector<Move, MAX_SEARCH_DEPTH>& moveLine, SearchMetrics& metrics, int depth=MAX_SEARCH_DEPTH) {
    if constexpr (type == SearchType::time) {
        int base = state->getTime(), increment = state->getInc();
        int moveTime = GameState::getMoveTime(base, increment);
        startTimer(moveTime);
    }

    int16_t score = 0, alpha = -VALUE_INFINITE, beta = VALUE_INFINITE;
    for(int ply = 1; ply <= depth; ++ply) {
        if(score == -VALUE_MATE || score == VALUE_MATE) //exit early if forced mate
            break;

        SearchMetrics currMetrics;
        FixedVector<Move, MAX_SEARCH_DEPTH> currMoveLine;

        auto start = std::chrono::high_resolution_clock::now();

        int16_t delta = ASPIRATION_WIDTH / 2;
        while(!stopRequested) {
            assert(alpha >= -VALUE_INFINITE && beta <= VALUE_INFINITE);
            score = alphaBeta(state, currMetrics, currMoveLine, alpha, beta, ply);
        
            if(score <= alpha) { //failed low
                alpha = std::max(-VALUE_INFINITE, score - delta);
                delta *= 2;
            } else if(score >= beta) { //failed high
                beta = std::min<int16_t>(VALUE_INFINITE, score + delta);
                delta *= 2;
            } else {
                alpha = score - ASPIRATION_WIDTH / 2, beta = score + ASPIRATION_WIDTH / 2;
                break;
            }
        }
       
        auto end = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration<double>(end - start).count();
        int nps = static_cast<int>(currMetrics.nodes / dur);
        metrics.nodes += currMetrics.nodes;

        if(stopRequested)
            break;
        
        moveLine = currMoveLine;
        std::cout << "info depth " << ply << " score cp " << score << " nodes " << currMetrics.nodes << " nps " << nps << " pv";
        for(Move move : moveLine)
            std::cout << " " << Board::getMoveString(move);
        std::cout << std::endl;
    } 

    if(moveLine.size() == 0) { //push default move
        MoveList moveList;
        state->getLegalMoves(moveList);
        moveLine.push_back(moveList[0]);
    }
}

template<SearchType type>
SearchMetrics Search::Search(GameState* state, int depth) {
    Board::PieceColor turn = state->getTurn();
    FixedVector<Move, MAX_SEARCH_DEPTH> moveLine;
    
    SearchMetrics metrics;
    if constexpr (type == SearchType::depth) {
        stopRequested = false;
        iterativeDeepening<SearchType::depth>(state, moveLine, metrics, depth);
    } else if constexpr (type == SearchType::time) {
        stopRequested = false;
        iterativeDeepening<SearchType::time>(state, moveLine, metrics);
    }

    std::cout << "bestmove " << Board::getMoveString(moveLine[0]);
    if(moveLine.size() > 1)
        std::cout << " ponder " << Board::getMoveString(moveLine[1]);
    std::cout << std::endl;

    return metrics;
}