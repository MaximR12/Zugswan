#pragma once

#include <iostream>
#include <cassert>
#include <array>
#include <vector>
#include <bit>
#include <bitset>
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

constexpr uint64_t PAWN_RANK = RANK_2 | RANK_7;

constexpr uint64_t NOT_A_FILE = 0xFEFEFEFEFEFEFEFEULL; 
constexpr uint64_t NOT_AB_FILE = 0xFCFCFCFCFCFCFCFCULL; 
constexpr uint64_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7FULL;
constexpr uint64_t NOT_GH_FILE = 0x3F3F3F3F3F3F3F3FULL;

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

constexpr uint16_t northOffset = 8;
constexpr uint16_t southOffset = -8;
constexpr uint16_t eastOffset = 1;
constexpr uint16_t westOffset = -1;
constexpr uint16_t northEastOffset = 9;
constexpr uint16_t northWestOffset = 7;
constexpr uint16_t southEastOffset = -7;
constexpr uint16_t southWestOffset = -9;

constexpr uint16_t northNorthEastOffset = 17;
constexpr uint16_t northEastEastOffset = 10;
constexpr uint16_t northNorthWestOffset = 15;
constexpr uint16_t northWestWestOffset = 6;
constexpr uint16_t southSouthEastOffset = -15;
constexpr uint16_t southEastEastOffset = -6;
constexpr uint16_t southSouthWestOffset = -17;
constexpr uint16_t southWestWestOffset = -10;

constexpr int MAX_LEGAL_MOVES = 256;
constexpr int NUM_PIECE_TYPES = 14;
constexpr int NUM_SQUARES = 64;
constexpr int NUM_TOTAL_DIRECTIONS = 16; //slider + knight directions
constexpr int NUM_SLIDER_DIRECTIONS = 8;
constexpr int NUM_KNIGHT_DIRECTIONS = 8;
constexpr int ROW_LEN = 8;

class Board {
private:
    std::array<uint64_t, NUM_PIECE_TYPES> m_pieceBB;
    uint64_t m_whiteEpTargets;
    uint64_t m_blackEpTargets;
    uint64_t m_emptyBB;
    uint64_t m_occupiedBB;

    std::array<std::array<uint64_t, NUM_SQUARES>, 2> m_pawnAttackTable;
    std::array<uint64_t, NUM_SQUARES> m_kingMoveTable;
    std::array<uint64_t, NUM_SQUARES> m_knightMoveTable;
    std::array<std::array<uint64_t, NUM_SQUARES>, NUM_SLIDER_DIRECTIONS> m_rayAttackTable; 

public:
    enum PieceType {
        whitePieces, blackPieces, 
        whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing,
        blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing,
        all, pawns, knights, bishops, rooks, queens, king,
        invalid
    };

    enum Directions {
        north, south, east, west, northWest, northEast, southWest, southEast, 
        northNorthEast, northEastEast, northNorthWest, northWestWest,
        southSouthEast, southEastEast, southSouthWest, southWestWest
    };

    Board();

    uint64_t getPieceSet(PieceType type) const { return m_pieceBB[type]; }
    uint64_t getBlackEpTargets() const { return m_blackEpTargets; }
    uint64_t getWhiteEpTargets() const { return m_whiteEpTargets; }
    uint64_t getOccupied() const { return m_occupiedBB; }
    uint64_t getEmpty() const { return m_emptyBB; }

    uint64_t getPawnSquareAttacks(uint16_t ind, PieceType color) const { assert(ind >= 0 && ind < NUM_SQUARES && color < 2); return m_pawnAttackTable[color][ind]; }
    uint64_t getKingMoves(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_kingMoveTable[ind]; }
    uint64_t getKnightMoves(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_knightMoveTable[ind]; }

    PieceType getPieceColor(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_pieceBB[whitePieces]&(1ULL<<ind) ? whitePieces : blackPieces; }
    PieceType getPieceType(uint16_t ind) const;

    void updateBB(PieceType type, uint64_t BB) { m_pieceBB[type] = BB; }
    void updateBlackEpTargets(uint64_t BB) { m_blackEpTargets = BB; }
    void updateWhiteEpTargets(uint64_t BB) { m_whiteEpTargets = BB; }
    void updateOccupiedBB(uint64_t BB) { m_occupiedBB = BB; }
    void updateEmptyBB(uint64_t BB) { m_emptyBB = BB; }

    static uint64_t getRayMoves(uint16_t ind, Directions dir);
    static uint64_t knightAttackTargets(uint64_t BB);
    static uint64_t kingAttackTargets(uint64_t BB);
    static uint64_t whitePawnTargets(uint64_t BB);
    static uint64_t blackPawnTargets(uint64_t BB);
    static uint64_t pawnAttackTargets(uint64_t pawns, Directions pawnDir); 
    static uint64_t wpAttackTargetsSafe(uint64_t BB, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween); //pin safe pawn attack targets
    static uint64_t bpAttackTargetsSafe(uint64_t BB, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween);
    static uint64_t pawnAttackTargetsSafe(uint64_t pawns, Directions pawnDir, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween); 
    static uint64_t pawnShift(uint64_t BB, Directions dir);

    static int64_t fullBoolMask(uint64_t BB) { return (int64_t)(BB | -BB) >> 63; }
    static int64_t nullBoolMask(uint64_t BB) { return ((int64_t)(BB) - 1) >> 63; }

    //ray directions
    static uint64_t shiftSouth(uint64_t BB) { return (BB >> 8); }
    static uint64_t shiftNorth(uint64_t BB) { return (BB << 8); }
    static uint64_t shiftEast(uint64_t BB) { return (BB << 1) & NOT_A_FILE; }
    static uint64_t shiftNorthEast(uint64_t BB) { return (BB << 9) & NOT_A_FILE; }
    static uint64_t shiftSouthEast(uint64_t BB) { return (BB >> 7) & NOT_A_FILE; }
    static uint64_t shiftWest(uint64_t BB) { return (BB >> 1) & NOT_H_FILE; }
    static uint64_t shiftNorthWest(uint64_t BB) { return (BB << 7) & NOT_H_FILE; }
    static uint64_t shiftSouthWest(uint64_t BB) { return (BB >> 9) & NOT_H_FILE; }

    //knight directions
    static uint64_t shiftNorthNorthEast(uint64_t BB) { return (BB << 17) & NOT_A_FILE; }
    static uint64_t shiftNorthEastEast(uint64_t BB) { return (BB << 10) & NOT_AB_FILE; }
    static uint64_t shiftNorthNorthWest(uint64_t BB) { return (BB << 15) & NOT_H_FILE; }
    static uint64_t shiftNorthWestWest(uint64_t BB) { return (BB << 6) & NOT_GH_FILE; }
    static uint64_t shiftSouthSouthEast(uint64_t BB) { return (BB >> 15) & NOT_A_FILE; }
    static uint64_t shiftSouthEastEast(uint64_t BB) { return (BB >> 6) & NOT_AB_FILE; }
    static uint64_t shiftSouthSouthWest(uint64_t BB) { return (BB >> 17) & NOT_H_FILE; }
    static uint64_t shiftSouthWestWest(uint64_t BB) { return (BB >> 10) & NOT_GH_FILE; }

    //fill each attack direction for sliders up to and including blockers
    static uint64_t northFill(uint64_t sliders, uint64_t empty);
    static uint64_t southFill(uint64_t sliders, uint64_t empty);
    static uint64_t eastFill(uint64_t sliders, uint64_t empty);
    static uint64_t westFill(uint64_t sliders, uint64_t empty);
    static uint64_t northEastFill(uint64_t sliders, uint64_t empty);
    static uint64_t northWestFill(uint64_t sliders, uint64_t empty);
    static uint64_t southEastFill(uint64_t sliders, uint64_t empty);
    static uint64_t southWestFill(uint64_t sliders, uint64_t empty);

    static uint16_t getDirectionOffset(Directions dir);
    static Directions getOppositeDirection(int dir);
    static Directions getOppositeDirection(Directions dir);
    static bool isNegative(Directions dir);

    static uint16_t bitScan(uint64_t BB, bool reverse);
    static uint16_t bitScanForward(uint64_t BB) { assert(BB != 0); return std::countr_zero(BB); }
    static uint16_t bitScanReverse(uint64_t BB) { assert(BB != 0); return 63 - std::countl_zero(BB); }
    static uint16_t serializeSingleBit(uint64_t BB) { return bitScanForward(BB); } //get square indices from bitboards
    static uint16_t serializeBitboard(uint64_t BB, std::array<uint16_t, NUM_SQUARES>& indBuf); //serialize into indBuf and return size

    static void printBitBoard(uint64_t BB);
};

inline consteval std::array<int16_t, NUM_TOTAL_DIRECTIONS> genDirectionOffsetTable() {
    constexpr std::array<int16_t, NUM_TOTAL_DIRECTIONS> directionOffsets {
        northOffset, southOffset, eastOffset, westOffset, northEastOffset, northWestOffset, 
        southEastOffset, southEastOffset, northNorthEastOffset, northEastEastOffset, northNorthWestOffset,
        northWestWestOffset, southSouthEastOffset, southEastEastOffset, southSouthWestOffset, southWestWestOffset
    };

    return directionOffsets;
}

inline consteval std::array<Board::Directions, NUM_TOTAL_DIRECTIONS> genOppositeDirectionTable() {
    constexpr std::array<Board::Directions, NUM_TOTAL_DIRECTIONS> oppositeDirections {
        Board::south, Board::north, Board::west, Board::east, Board::southEast, Board::southWest,
        Board::northEast, Board::northWest, Board::southSouthWest, Board::southWestWest, Board::southSouthEast,
        Board::southEastEast, Board::northNorthWest, Board::northWestWest, Board::northNorthEast, Board::northEastEast,
    };

    return oppositeDirections;
}

inline constinit const std::array<int16_t, NUM_TOTAL_DIRECTIONS> directionOffsetTable {genDirectionOffsetTable()}; 
inline constinit const std::array<Board::Directions, NUM_TOTAL_DIRECTIONS> oppositeDirectionTable {genOppositeDirectionTable()}; 

inline uint16_t Board::getDirectionOffset(Directions dir) { 
    assert(dir >= 0 && dir <= NUM_TOTAL_DIRECTIONS);
    return directionOffsetTable[dir];
}

inline Board::Directions Board::getOppositeDirection(Directions dir) {
    assert(dir >= 0 && dir <= NUM_TOTAL_DIRECTIONS);
    return oppositeDirectionTable[dir];
}

inline Board::Directions Board::getOppositeDirection(int dir) {
    assert(dir >= 0 && dir <= NUM_TOTAL_DIRECTIONS);
    return oppositeDirectionTable[dir];
}

inline bool Board::isNegative(Directions dir) {
    return directionOffsetTable[dir] < 0;
}

inline uint64_t Board::pawnShift(uint64_t BB, Directions dir) {
    uint64_t negDirMask = 0ULL - static_cast<uint64_t>(isNegative(dir));
    return (BB << directionOffsetTable[dir] & ~negDirMask) | (BB >> -directionOffsetTable[dir] & negDirMask);
}