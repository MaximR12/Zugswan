#pragma once

#include "move.hpp"
#include "board.hpp"
#include "tables.hpp"
#include <vector> 

class MoveGen {
private:
    Board* m_board;
    Tables* const m_tables;

public:
    MoveGen(Board* board, Tables* tables) : m_board{board}, m_tables{tables} { }

    uint16_t getLegalMoves(Board::PieceColor color, std::array<Move, MAX_LEGAL_MOVES>& moveBuf) const;
    void updateBoard(Board* board) { m_board = board; }
};