#pragma once

#include "move.hpp"
#include "board.hpp"
#include "tables.hpp"
#include "fixedvector.hpp"
#include <vector> 

class MoveGen {
private:
    Board* m_board;
    Tables* const m_tables;

public:
    MoveGen(Board* board, Tables* tables) : m_board{board}, m_tables{tables} { }

    void getLegalMoves(Board::PieceColor color, FixedVector<Move, MAX_LEGAL_MOVES>& moveList) const;
    void updateBoard(Board* board) { m_board = board; }
};