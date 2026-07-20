#pragma once

#include <iostream>
#include <vector>
#include "board.hpp"
#include "movegen.hpp"
#include "move.hpp"

constexpr int UNDEFINED_SQUARE = 65;
constexpr int UNDO_STACK_SIZE = 256;

struct StateInfo {
    uint64_t epTarget;
    uint64_t oppEpTarget;
    Board::PieceType captureType;
    uint16_t halfMoveClock;
    bool kingCastleRights;
    bool queenCastleRights;
};

class GameState {
private:
    FixedVector<StateInfo, UNDO_STACK_SIZE> m_undoStack;
    FixedVector<Move, MAX_LEGAL_MOVES> m_legalMoves;
    
    uint64_t m_zobrist;
    Board m_board;
    Board::PieceColor m_turn;
    bool m_inCheck;

    int m_whiteTime;
    int m_whiteInc;
    int m_blackTime;
    int m_blackInc;

    void movePiece(Board::PieceType type, Board::PieceColor color, uint64_t fromToBB, uint16_t from, uint16_t to);
    void removePiece(Board::PieceType type, Board::PieceColor color, uint64_t pieceBB, uint16_t square);
    void addPiece(Board::PieceType type, Board::PieceColor color, uint64_t pieceBB, uint16_t square);
    void updateCastleRights(Board::PieceColor fromColor, Board::PieceColor oppColor, uint64_t fromBB, uint64_t toBB);

public:
    GameState();

    void makeMove(Move move);
    void unmakeMove(Move move);

    void switchTurn() { m_turn = Board::getOppositeColor(m_turn); m_zobrist ^= Tables::ZTable.blackSide; }
    const Board::PieceColor getTurn() const { return m_turn; }

    void loadPosition(std::string fen);
    void loadStartPos() { loadPosition(START_FEN); }
    void moveFromList(std::vector<std::string>& moveList);

    Board* getBoard() { return &m_board; }
    uint64_t getZobrist() { return m_zobrist; }

    bool inCheck() const { return m_inCheck; }

    void updateLegalMoves() { m_legalMoves.clear(); MoveGen::getLegalMoves(&m_board, m_turn, m_legalMoves); }
    void getLegalMoves(FixedVector<Move, MAX_LEGAL_MOVES>& moveList) { m_inCheck = MoveGen::getLegalMoves(&m_board, m_turn, moveList); };

    void updateTime(int wTime, int bTime, int wInc=0, int bInc=0) { m_whiteTime = wTime; m_whiteInc = wInc; m_blackTime = bTime; m_blackInc = bInc; }
    int getTime() { return m_turn == Board::white ? m_whiteTime : m_blackTime; }
    int getInc() { return m_turn == Board::white ? m_whiteInc : m_blackInc; }
    
    static uint16_t getRow(uint16_t sq) { return ROW_LEN - sq / ROW_LEN; }
    static uint16_t getCol(uint16_t sq) { return sq % ROW_LEN; }

    static uint64_t getZobrist(Board* board, Board::PieceColor turn);
    static int getMoveTime(int base, int increment) { return base / 20 + increment / 2; }
};