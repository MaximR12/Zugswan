#pragma once

#include <iostream>
#include <vector>
#include "Board.h"
#include "MoveGen.h"
#include "Engine.h"
#include "Move.h"
#include "Piece.h"

constexpr int UNDEFINED_SQUARE = 65;
constexpr int INIT_STACK_SIZE = 256;

class GameState {
private:
    const Board::PieceColor m_playerTurn;
    Board::PieceColor m_turn;
    std::vector<Board> m_boardStack;
    Board* m_board;

    Tables m_tables;
    MoveGen m_moveGen;
    Engine m_engine;

    uint16_t m_numLegalMoves;
    std::array<Move, MAX_LEGAL_MOVES> m_currLegalMoves;
    std::vector<Move> m_moveList;

    uint16_t m_selectedSquare;
    uint64_t m_highlightedSquares;
    
public:
    GameState();

    void makeMove(Move move);
    void unMakeMove();
    void playEngineMove();

    void switchTurn() { m_turn = Board::getOppositeColor(m_turn); }
    const Board::PieceColor getTurn() { return m_turn; }
    const Board::PieceColor playerTurn() { return m_playerTurn; }

    
    void handleClick(int square);
    uint16_t getSelected() const { return m_selectedSquare; } 
    void setHighlighted(uint16_t sq) { m_highlightedSquares |= (1ULL << sq); }

    void updateBoard() { m_board = &m_boardStack.back(); m_moveGen.updateBoard(m_board); m_engine.updateBoard(m_board); }
    void updateLegalMoves() { m_numLegalMoves = m_moveGen.getLegalMoves(m_playerTurn, m_currLegalMoves); }
    uint16_t getLegalMoves(std::array<Move, MAX_LEGAL_MOVES>& moveBuf) const { return m_moveGen.getLegalMoves(m_turn, moveBuf); };
    std::vector<Piece> getPieceList() const;

    static uint16_t getRow(uint16_t sq) { return ROW_LEN - sq / ROW_LEN; }
    static uint16_t getCol(uint16_t sq) { return sq % ROW_LEN; }
};