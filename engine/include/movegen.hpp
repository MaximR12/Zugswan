#pragma once

#include "move.hpp"
#include "board.hpp"
#include "tables.hpp"
#include "fixedvector.hpp"
#include <vector> 

enum class GenType {
    quiet, capture
};

class GameState;

namespace MoveGen {

    //return true if in check
    bool getLegalMoves(GameState& state, Board::PieceColor color, MoveList& moveList);

}