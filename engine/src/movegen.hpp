#pragma once

#include "move.hpp"
#include "board.hpp"
#include "tables.hpp"
#include "fixedvector.hpp"
#include <vector> 

enum class GenType {
    capture, all
};

namespace MoveGen {
    //return true if in check
    template<GenType genType>
    bool getLegalMoves(Board& board, MoveList& moveList, uint16_t ply);

    template bool getLegalMoves<GenType::capture>(Board& board, MoveList& moveList, uint16_t ply);
    template bool getLegalMoves<GenType::all>(Board& board, MoveList& moveList, uint16_t ply);
}