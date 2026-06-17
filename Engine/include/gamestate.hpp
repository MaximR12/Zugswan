#pragma once

#include <iostream>
#include <vector>
#include "board.hpp"
#include "movegen.hpp"
#include "move.hpp"

constexpr char START_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

constexpr int UNDEFINED_SQUARE = 65;
constexpr int INIT_STACK_SIZE = 256;

class GameState {
private:
    Board::PieceColor m_turn;
    std::vector<Board> m_boardStack;
    Board* m_board;

    Tables m_tables;
    MoveGen m_moveGen;

    uint16_t m_numLegalMoves;
    std::array<Move, MAX_LEGAL_MOVES> m_currLegalMoves;
    std::vector<Move> m_moveList;

    uint16_t m_selectedSquare;
    
public:
    GameState();

    void makeMove(Move move);
    void unMakeMove();

    void switchTurn() { m_turn = Board::getOppositeColor(m_turn); }
    const Board::PieceColor getTurn() const { return m_turn; }
    
    void handleClick(int square);
    uint16_t getSelected() const { return m_selectedSquare; } 

    void loadPosition(std::string fen);
    void loadStartPos() { loadPosition(START_FEN); }
    void moveFromList(std::vector<std::string>& moveList);

    void updateBoard() { m_board = &m_boardStack.back(); m_moveGen.updateBoard(m_board); }
    Board* getBoard() { return m_board; }

    void updateLegalMoves() { m_numLegalMoves = m_moveGen.getLegalMoves(m_turn, m_currLegalMoves); }
    uint16_t getLegalMoves(std::array<Move, MAX_LEGAL_MOVES>& moveBuf) const { return m_moveGen.getLegalMoves(m_turn, moveBuf); };
    
    static uint16_t getRow(uint16_t sq) { return ROW_LEN - sq / ROW_LEN; }
    static uint16_t getCol(uint16_t sq) { return sq % ROW_LEN; }
};