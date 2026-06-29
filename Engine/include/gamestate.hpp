#pragma once

#include <iostream>
#include <vector>
#include "board.hpp"
#include "movegen.hpp"
#include "move.hpp"

constexpr int UNDEFINED_SQUARE = 65;
constexpr int INIT_STACK_SIZE = 256;

enum class State {
    draw, whiteMate, blackMate, inProgress
};

class GameState {
private:
    std::vector<Board> m_boardStack;
    Board* m_board;

    Board::PieceColor m_turn;
    State m_state;

    Tables m_tables;
    MoveGen m_moveGen;

    FixedVector<Move, MAX_LEGAL_MOVES> m_legalMoves;
    
public:
    GameState();

    void makeMove(Move move);
    void unMakeMove();

    void switchTurn() { m_turn = Board::getOppositeColor(m_turn); }
    const Board::PieceColor getTurn() const { return m_turn; }

    void loadPosition(std::string fen);
    void loadStartPos() { loadPosition(START_FEN); }
    void moveFromList(std::vector<std::string>& moveList);

    void updateBoard() { m_board = &m_boardStack.back(); m_moveGen.updateBoard(m_board); }
    Board* getBoard() { return m_board; }
    State getState() { return m_state; }
    State determineEndState(); //calculate end state assuming there are no legal moves

    void updateLegalMoves() { m_moveGen.getLegalMoves(m_turn, m_legalMoves); }
    void getLegalMoves(FixedVector<Move, MAX_LEGAL_MOVES>& moveList) const { return m_moveGen.getLegalMoves(m_turn, moveList); };
    
    static uint16_t getRow(uint16_t sq) { return ROW_LEN - sq / ROW_LEN; }
    static uint16_t getCol(uint16_t sq) { return sq % ROW_LEN; }
};