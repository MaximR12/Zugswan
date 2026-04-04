#pragma once

#include <iostream>
#include <cassert>
#include <array>
#include <vector>
#include <bit>
#include "stdint.h"

/*
Board class encapsulating piece bitboards using little endian rank file mappings
  H  G  F  E  D  C  B  A
8 63 62 61 60 59 58 57 56   
7 55 54 53 52 51 50 49 48
6 47 46 45 44 43 42 41 40
5 39 38 37 36 35 34 33 32
4 31 30 29 28 27 26 25 24
3 23 22 21 20 19 18 17 16
2 15 14 13 12 11 10  9  8
1  7  6  5  4  3  2  1  0   
*/

    constexpr uint64_t UNIVERSE = 0xFFFFFFFFFFFFFFFFULL;
    constexpr uint64_t EMPTY = 0x0000000000000000ULL;

    constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
    constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
    constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
    constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
    constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
    constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
    constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
    constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

    constexpr uint64_t NOT_A_FILE = 0xFEFEFEFEFEFEFEFEULL; 
    constexpr uint64_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7FULL;

    constexpr uint64_t WHITE_KNIGHTS = 0x0000000000000042ULL; 
    constexpr uint64_t BLACK_KNIGHTS = 0x4200000000000000ULL;
    constexpr uint64_t WHITE_BISHOPS = 0x0000000000000024ULL; 
    constexpr uint64_t BLACK_BISHOPS = 0x2400000000000000ULL;
    constexpr uint64_t WHITE_ROOKS = 0x0000000000000081ULL; 
    constexpr uint64_t BLACK_ROOKS = 0x8100000000000000ULL;    
    constexpr uint64_t WHITE_QUEENS = 0x0000000000000008ULL; 
    constexpr uint64_t BLACK_QUEENS = 0x0800000000000000ULL;
    constexpr uint64_t WHITE_KING = 0x0000000000000010ULL; 
    constexpr uint64_t BLACK_KING = 0x1000000000000000ULL;

    constexpr int MAX_LEGAL_MOVES = 256;
    constexpr int NUM_PIECE_TYPES = 14;
    constexpr int NUM_SQUARES = 64;
    constexpr int NUM_DIRECTIONS = 8;
    constexpr int ROW_LEN = 8;

class Board {
private:
    void initMoveTables();

    std::array<uint64_t, NUM_PIECE_TYPES> m_pieceBB;
    uint64_t m_emptyBB;
    uint64_t m_occupiedBB;

    std::array<uint64_t, NUM_SQUARES> m_whitePawnAttackTable;
    std::array<uint64_t, NUM_SQUARES> m_blackPawnAttackTable;
    std::array<uint64_t, NUM_SQUARES> m_kingMoveTable;
    std::array<uint64_t, NUM_SQUARES> m_knightMoveTable;
    std::array<uint64_t, NUM_SQUARES> m_bishopMoveTable;
    std::array<uint64_t, NUM_SQUARES> m_rookMoveTable;
    std::array<uint64_t, NUM_SQUARES> m_queenMoveTable;
    std::array<std::array<uint64_t, NUM_SQUARES>, NUM_DIRECTIONS> m_rayAttackTable; 

public:
    enum PieceType {
        whitePieces, blackPieces, 
        whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing,
        blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing,
        invalid
    };

    enum Directions {
        north, south, east, west, northWest, northEast, southWest, southEast
    };

    Board() {
        m_pieceBB[whitePieces] = RANK_1 | RANK_2;
        m_pieceBB[blackPieces] = RANK_7 | RANK_8;
        m_pieceBB[whitePawns] = RANK_2; 
        m_pieceBB[blackPawns] = RANK_7;
        m_pieceBB[whiteKnights] = WHITE_KNIGHTS; 
        m_pieceBB[blackKnights] = BLACK_KNIGHTS; 
        m_pieceBB[whiteBishops] = WHITE_BISHOPS; 
        m_pieceBB[blackBishops] = BLACK_BISHOPS; 
        m_pieceBB[whiteRooks] = WHITE_ROOKS; 
        m_pieceBB[blackRooks] = BLACK_ROOKS; 
        m_pieceBB[whiteQueens] = WHITE_QUEENS; 
        m_pieceBB[blackQueens] = BLACK_QUEENS; 
        m_pieceBB[whiteKing] = WHITE_KING; 
        m_pieceBB[blackKing] = BLACK_KING; 

        m_occupiedBB = m_pieceBB[whitePieces] | m_pieceBB[blackPieces];
        m_emptyBB = ~m_occupiedBB;
    
        initMoveTables();
    };

    uint64_t getPieceSet(PieceType type) const { return m_pieceBB[type]; }
    uint64_t getOccupied() const { return m_occupiedBB; }
    uint64_t getEmpty() const { return m_emptyBB; }

    uint64_t getWhitePawnAttacks(uint16_t ind) const { assert(ind >= ROW_LEN && ind < NUM_SQUARES-ROW_LEN); return m_whitePawnAttackTable[ind]; }
    uint64_t getBlackPawnAttacks(uint16_t ind) const { assert(ind >= ROW_LEN && ind < NUM_SQUARES-ROW_LEN); return m_blackPawnAttackTable[ind]; }
    uint64_t getKingMoves(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_kingMoveTable[ind]; }
    uint64_t getKnightMoves(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_knightMoveTable[ind]; }

    PieceType getPieceColor(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_pieceBB[whitePieces]&(1ULL<<ind) ? whitePieces : blackPieces; }
    PieceType getPieceType(uint16_t ind) const;

    void updateBB(PieceType type, uint64_t BB) { m_pieceBB[type] = BB; }
    void updateOccupiedBB(uint64_t BB) { m_occupiedBB = BB; }
    void updateEmptyBB(uint64_t BB) { m_emptyBB = BB; }

    static uint64_t shiftSouth(uint64_t BB) { return (BB >> 8); }
    static uint64_t shiftNorth(uint64_t BB) { return (BB << 8); }
    static uint64_t shiftEast(uint64_t BB) { return (BB << 1) & NOT_A_FILE; }
    static uint64_t shiftNortheast(uint64_t BB) { return (BB << 9) & NOT_A_FILE; }
    static uint64_t shiftSoutheast(uint64_t BB) { return (BB >> 7) & NOT_A_FILE; }
    static uint64_t shiftWest(uint64_t BB) { return (BB >> 1) & NOT_H_FILE; }
    static uint64_t shiftNorthwest(uint64_t BB) { return (BB << 7) & NOT_H_FILE; }
    static uint64_t shiftSouthwest(uint64_t BB) { return (BB >> 9) & NOT_H_FILE; }

    static uint16_t bitScanForward(uint64_t BB) { assert(BB != 0); return std::countr_zero(BB); }
    static uint16_t serializeSingleBit(uint64_t BB) { return bitScanForward(BB); } //get square indices from bitboards
    static uint16_t serializeBitboard(uint64_t BB, std::array<uint16_t, NUM_SQUARES>& indBuf);
};
