#pragma once

#include "move.hpp"
#include "board.hpp"
#include "tables.hpp"
#include "fixedvector.hpp"
#include <vector> 

enum class GenType {
    quiet, capture, evasion
};

namespace MoveGen {

    //return true if in check
    bool getLegalMoves(Board* board, Board::PieceColor color, FixedVector<Move, MAX_LEGAL_MOVES>& moveList);

}