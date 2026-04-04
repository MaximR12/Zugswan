#pragma once

#include "Move.h"
#include "Board.h"
#include <vector> 

enum class Turn {  
    WHITE,
    BLACK
};

class MoveGen {
private:
    Board* const m_board;

public:
    MoveGen(Board* board) : m_board{board} { }

    std::vector<Move> getLegalMoves(Turn turn);
};