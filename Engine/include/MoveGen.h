#pragma once

#include "Move.h"
#include "Board.h"
#include <vector> 

class MoveGen {
private:
    Board* const m_board;

public:
    MoveGen(Board* board) : m_board{board} { }

    std::vector<Move> getLegalMoves(Board::PieceColor color);
};