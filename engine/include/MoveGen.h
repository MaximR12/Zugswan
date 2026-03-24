#pragma once

#include "Move.h"
#include "Board.h"
#include "stdint.h"
#include <vector> 
#include <cassert>

namespace Game {
    struct MoveArr {
        std::array<Move, MAX_LEGAL_MOVES> arr;
        size_t size;

        MoveArr() : size{0}, arr{} { }
        void append(uint16_t flags, uint16_t from, uint16_t to) { assert(size < MAX_LEGAL_MOVES); arr[size++] = Move(flags, from, to); }
    };

    class MoveGen {
    private:
        Board* const m_board;
    
    public:
        MoveGen(Board* board) : m_board{board} { }

        MoveArr getLegalMoves();
    };
}