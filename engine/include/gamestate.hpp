#pragma once

#include <iostream>
#include <vector>
#include "board.hpp"
#include "movegen.hpp"
#include "move.hpp"

struct StateInfo {
    uint64_t epTarget;
    uint64_t oppEpTarget;
    Board::PieceType captureType;
    uint16_t halfMoveClock;
    uint16_t lastReversible;
    bool kingCastleRights;
    bool queenCastleRights;
};

class GameState {
private:
    FixedVector<uint64_t, MAX_GAME_LENGTH> m_hashList;
    FixedVector<StateInfo, MAX_GAME_LENGTH> m_undoStack;
    MoveList m_legalMoves;
    
    std::array<int16_t, 2> m_pstScores;
    uint64_t m_zobrist;
    uint16_t m_lastReversible;
    uint16_t m_ply;
    Board m_board;
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
    void makeNullMove();
    void unmakeNullMove();

    void switchTurn() { m_board.switchTurn(); m_zobrist ^= Tables::ZTable.blackSide; }
    const Board::PieceColor getTurn() const { return m_board.getTurn(); }

    void loadPosition(std::string fen);
    void loadStartPos() { loadPosition(START_FEN); }
    void moveFromList(std::vector<std::string>& moveList);

    Board* getBoard() { return &m_board; }
    uint64_t getZobrist() { return m_zobrist; }
    int16_t getPst(Board::PieceColor color) { return m_pstScores[color]; }
    int16_t getSEE(Move move) const { return m_board.staticExchangeEvaluation(move); }
    uint16_t getPly() const { return m_ply; }

    uint16_t getHalfMoveClock() { return m_board.getHalfMoveClock(); }
    void setLastReversible() { m_lastReversible = m_ply; }

    bool inCheck() const { return m_inCheck; }
    bool isRepetition() const;
    bool promotionPossible() const { return m_board.promotionPossible(); }
    bool shouldNullSearch() const { return !(m_inCheck || m_board.pawnEndgame()); };
    bool shouldReduce(Move move) const;

    void updateLegalMoves() { m_legalMoves.clear(); MoveGen::getLegalMoves(m_board, m_legalMoves, m_ply); }
    void getLegalMoves(MoveList& moveList) { m_inCheck = MoveGen::getLegalMoves(m_board, moveList, m_ply); };

    void updateTime(int wTime, int bTime, int wInc=0, int bInc=0) { m_whiteTime = wTime; m_whiteInc = wInc; m_blackTime = bTime; m_blackInc = bInc; }
    int getTime() { return m_board.getTurn() == Board::white ? m_whiteTime : m_blackTime; }
    int getInc() { return m_board.getTurn() == Board::white ? m_whiteInc : m_blackInc; }
    
    static uint16_t getRow(uint16_t sq) { return ROW_LEN - sq / ROW_LEN; }
    static uint16_t getCol(uint16_t sq) { return sq % ROW_LEN; }

    static uint64_t calculateZobrist(Board* board);
    static int16_t calculatePstScore(Board* board, Board::PieceColor turn);
    static int getMoveTime(int base, int increment) { return base / 20 + increment / 2; }
};