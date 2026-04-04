#pragma once

#include <iostream>
#include <vector>
#include "Board.h"
#include "MoveGen.h"
#include "Engine.h"
#include "Move.h"
#include "Piece.h"

constexpr int UNDEFINED_SQUARE = 65;

class GameState {
private:
    const Turn m_playerTurn;
    Turn m_turn;
    Board m_board;
    MoveGen m_moveGen;
    Engine m_engine;

    std::vector<Move> m_currLegalMoves;
    std::vector<Move> m_moveList;

    uint16_t m_selectedSquare;
    uint64_t m_highlightedSquares;
    
public:
    GameState() : m_board{}, m_playerTurn{Turn::WHITE}, m_turn{Turn::WHITE}, m_moveGen{&m_board}, m_engine{&m_board, &m_moveGen}, m_currLegalMoves{}, m_selectedSquare{UNDEFINED_SQUARE} { 
        m_currLegalMoves.reserve(MAX_LEGAL_MOVES); 
        m_currLegalMoves = m_moveGen.getLegalMoves(m_playerTurn); 
    }

    void makeMove(Move move);
    void unMakeMove(Move move);

    void switchTurn() { m_turn = m_turn == Turn::WHITE ? Turn::BLACK : Turn::WHITE; }
    const Turn getTurn() { return m_turn; }
    const Turn playerTurn() { return m_playerTurn; }

    void playEngineMove();

    void handleClick(int square);

    uint16_t getSelected() const { return m_selectedSquare; } 
    void setHighlighted(uint16_t sq) { m_highlightedSquares |= (1ULL << sq); }

    void updateLegalMoves() { m_currLegalMoves = m_moveGen.getLegalMoves(m_playerTurn); }
    const std::vector<Move>& getLegalMoves() const { return m_currLegalMoves; }
    std::vector<Piece> getPieceList() const;

    static uint16_t getRow(uint16_t sq) { return ROW_LEN - sq / ROW_LEN; }
    static uint16_t getCol(uint16_t sq) { return sq % ROW_LEN; }
};