#pragma once

#include <iostream>
#include <vector>
#include "Board.h"
#include "MoveGen.h"
#include "Move.h"
#include "Piece.h"

namespace Game {
    enum class State {  
        WHITE_TO_MOVE,
        BLACK_TO_MOVE,
        GAME_OVER
    };

    class GameState {
    private:
        State m_state;
        Board m_board;
        MoveGen m_moveGen;
        std::vector<Move> m_moveList;
        
    public:
        GameState() : m_board{}, m_state{State::WHITE_TO_MOVE}, m_moveGen{&m_board} { }

        void makeMove(Move move);
        void unMakeMove(Move move);

        void handleClick(int row, int col);

        std::vector<Piece> getPieceList() const;
    };
}