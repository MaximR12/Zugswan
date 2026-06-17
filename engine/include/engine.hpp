#pragma once

#include "move.hpp"
#include "gamestate.hpp"

class Engine {
private:
    GameState* m_state;
    
public:
    Engine(GameState* board) : m_state{board} { }

    const Move getTopMove(const Board::PieceColor turn);
};