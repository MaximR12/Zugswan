#pragma once

#include "Move.h"
#include "MoveGen.h"


class Engine {
private:
    Board* const m_board;
    MoveGen* const m_moveGen;
    
public:
    Engine(Board* board, MoveGen* moveGen) : m_board{board}, m_moveGen{moveGen} { }

    const Move getTopMove(const Turn turn);
};