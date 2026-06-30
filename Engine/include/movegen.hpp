#pragma once

#include "move.hpp"
#include "board.hpp"
#include "tables.hpp"
#include "fixedvector.hpp"
#include <vector> 

namespace MoveGen {

    void getLegalMoves(Board* board, Board::PieceColor color, FixedVector<Move, MAX_LEGAL_MOVES>& moveList);

}