#pragma once

#include <iostream>
#include <vector>
#include "board.hpp"
#include "movegen.hpp"
#include "move.hpp"

constexpr int UNDEFINED_SQUARE = 65;
constexpr int UNDO_STACK_SIZE = 256;

struct StateInfo {
    uint64_t epTarget;
    Board::PieceType captureType;
    uint16_t halfMoveClock;
    bool kingCastleRights;
    bool queenCastleRights;
};

enum class State {
    draw, whiteMate, blackMate, inProgress
};

class GameState {
private:
    FixedVector<StateInfo, UNDO_STACK_SIZE> m_undoStack;
    Board m_board;

    Board::PieceColor m_turn;
    State m_state;

    FixedVector<Move, MAX_LEGAL_MOVES> m_legalMoves;
    
public:
    GameState();

    void makeMove(Move move);
    void unMakeMove(Move move);

    void switchTurn() { m_turn = Board::getOppositeColor(m_turn); }
    const Board::PieceColor getTurn() const { return m_turn; }

    void loadPosition(std::string fen);
    void loadStartPos() { loadPosition(START_FEN); }
    void moveFromList(std::vector<std::string>& moveList);

    Board* getBoard() { return &m_board; }
    State getState() { return m_state; }
    State determineEndState(); //calculate end state assuming there are no legal moves

    void updateLegalMoves() { m_legalMoves.clear(); MoveGen::getLegalMoves(&m_board, m_turn, m_legalMoves); }
    void getLegalMoves(FixedVector<Move, MAX_LEGAL_MOVES>& moveList) { return MoveGen::getLegalMoves(&m_board, m_turn, moveList); };
    
    static uint16_t getRow(uint16_t sq) { return ROW_LEN - sq / ROW_LEN; }
    static uint16_t getCol(uint16_t sq) { return sq % ROW_LEN; }
};