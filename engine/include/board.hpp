#pragma once

#include <iostream>
#include <cassert>
#include <array>
#include <vector>
#include <bit>
#include <bitset>
#include "stdint.h"
#include "move.hpp"

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

constexpr char START_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
constexpr uint64_t A_FILE = ~NOT_A_FILE;
constexpr uint64_t NOT_AB_FILE = 0xFCFCFCFCFCFCFCFCULL; 
constexpr uint64_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7FULL;
constexpr uint64_t NOT_GH_FILE = 0x3F3F3F3F3F3F3F3FULL;
constexpr uint64_t NOT_LAST_RANK = 0x00FFFFFFFFFFFF00ULL;
constexpr uint64_t LAST_RANK = 0xFF000000000000FFULL;

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

constexpr int16_t northOffset = 8;
constexpr int16_t southOffset = -8;
constexpr int16_t eastOffset = 1;
constexpr int16_t westOffset = -1;
constexpr int16_t northEastOffset = 9;
constexpr int16_t northWestOffset = 7;
constexpr int16_t southEastOffset = -7;
constexpr int16_t southWestOffset = -9;

constexpr int16_t northNorthEastOffset = 17;
constexpr int16_t northEastEastOffset = 10;
constexpr int16_t northNorthWestOffset = 15;
constexpr int16_t northWestWestOffset = 6;
constexpr int16_t southSouthEastOffset = -15;
constexpr int16_t southEastEastOffset = -6;
constexpr int16_t southSouthWestOffset = -17;
constexpr int16_t southWestWestOffset = -10;

constexpr int MAX_LEGAL_MOVES = 256;
constexpr int NUM_PIECE_TYPES = 8;
constexpr int NUM_SQUARES = 64;
constexpr int NUM_TOTAL_DIRECTIONS = 16; //slider + knight directions
constexpr int NUM_SLIDER_DIRECTIONS = 8;
constexpr int NUM_KNIGHT_DIRECTIONS = 8;
constexpr int NUM_ORTHOGONAL_DIRECTIONS = 4;
constexpr int NUM_MASK_TYPES = 10;
constexpr int ROW_LEN = 8;

class Board {
private:
    uint64_t zobrist;
    std::array<std::array<uint64_t, NUM_PIECE_TYPES>, 2> m_pieceBB;
    std::array<uint64_t, 2> m_enPassantTargets;
    uint64_t m_emptyBB;
    uint64_t m_occupiedBB;
    uint16_t m_halfMoveClock;
    uint16_t m_fullMoveCounter;
    std::array<bool, 2> m_kingCastleRights;
    std::array<bool, 2> m_queenCastleRights;

public:
    enum PieceColor {
        white, black
    };

    enum PieceType {
        pawns, knights, bishops, rooks, queens, king,
        all, invalid
    };

    enum CastleRights : uint8_t {
        noRights,
        whiteKing,
        whiteQueen = whiteKing << 1,
        blackKing = whiteQueen << 1,
        blackQueen = blackKing << 1,

        kingRights = whiteKing | blackKing,
        queenRights = whiteQueen | blackQueen,

        cAll = kingRights | queenRights,

        numCastleRights = 16
    };

    enum Directions {
        north, south, east, west, northWest, northEast, southWest, southEast, 
        northNorthEast, northEastEast, northNorthWest, northWestWest,
        southSouthEast, southEastEast, southSouthWest, southWestWest
    };

    enum SliderRays {
        hor, ver, diag, anti
    };

    Board() { }

    uint64_t getPieceSet(PieceType type, PieceColor color) const { return m_pieceBB[color][type]; }
    uint64_t getEnPassantTarget(PieceColor color) const { return m_enPassantTargets[color]; }
    uint8_t getCastleRights() const  
        { return (m_kingCastleRights[Board::white]<<0) | (m_queenCastleRights[Board::white]<<1) | (m_kingCastleRights[Board::black]<<2) | (m_queenCastleRights[Board::black]<<3); }
    bool getKingCastleRights(PieceColor color) const { return m_kingCastleRights[color]; }
    bool getQueenCastleRights(PieceColor color) const { return m_queenCastleRights[color]; }
    uint64_t getOccupied() const { return m_occupiedBB; }
    uint64_t getEmpty() const { return m_emptyBB; }
    uint16_t getHalfMoveClock() const { return m_halfMoveClock; }
    PieceColor getPieceColor(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_pieceBB[white][all]&(1ULL<<ind) ? white : black; }
    PieceType getPieceType(uint16_t ind) const;

    void updateBB(PieceType type, PieceColor color, uint64_t BB) { m_pieceBB[color][type] = BB; }
    void updateEnPassantTargets(PieceColor color, uint64_t BB) { m_enPassantTargets[color] = BB; }
    void updateKingCastleRights(PieceColor color, bool castleRights) { m_kingCastleRights[color] = castleRights; }
    void updateQueenCastleRights(PieceColor color, bool castleRights) { m_queenCastleRights[color] = castleRights; }
    void updateHalfMoveClock(uint16_t halfMoveClock) { m_halfMoveClock = halfMoveClock; }
    void incrementHalfMoveClock() { ++m_halfMoveClock; }
    void resetHalfMoveClock() { m_halfMoveClock = 0; }
    void updateOccupiedBB(uint64_t BB) { m_occupiedBB = BB; }
    void updateEmptyBB(uint64_t BB) { m_emptyBB = BB; }

    void clearPosition();
    PieceColor loadPosition(std::string& fen); //return turn, halfmove and fullmove info for gamestate

    static uint64_t knightAttackTargets(uint64_t BB);
    static uint64_t kingAttackTargets(uint64_t BB);
    static uint64_t whitePawnTargets(uint64_t BB);
    static uint64_t blackPawnTargets(uint64_t BB);

    static int64_t fullBoolMask(bool cond) { return 0ULL - static_cast<uint64_t>(cond); }
    static int64_t fullBoolMask(uint64_t BB) { return 0ULL - static_cast<uint64_t>(BB != 0); }
    static int64_t nullBoolMask(uint64_t BB) { return 0ULL - static_cast<uint64_t>(BB == 0); }

    template<Board::Directions dir>
    static constexpr uint64_t shift(uint64_t BB) {
        return dir == Board::south ? (BB >> 8)
            : dir == Board::north ? (BB << 8)
            : dir == Board::east ? (BB << 1) & NOT_A_FILE
            : dir == Board::west ? (BB >> 1) & NOT_H_FILE
            : dir == Board::northEast ? (BB << 9) & NOT_A_FILE
            : dir == Board::southEast ? (BB >> 7) & NOT_A_FILE
            : dir == Board::northWest ? (BB << 7) & NOT_H_FILE
            : dir == Board::southWest ? (BB >> 9) & NOT_H_FILE
            : dir == Board::northNorthEast ? (BB << 17) & NOT_A_FILE
            : dir == Board::northEastEast ? (BB << 10) & NOT_AB_FILE
            : dir == Board::northNorthWest ? (BB << 15) & NOT_H_FILE
            : dir == Board::northWestWest ? (BB << 6) & NOT_GH_FILE
            : dir == Board::southSouthEast ? (BB >> 15) & NOT_A_FILE
            : dir == Board::southEastEast ? (BB >> 6) & NOT_AB_FILE
            : dir == Board::southSouthWest ? (BB >> 17) & NOT_H_FILE
            : dir == Board::southWestWest ? (BB >> 10) & NOT_GH_FILE
            : EMPTY;
    }   

    //fill each attack direction for sliders up to and including blockers
    static uint64_t northFill(uint64_t sliders, uint64_t empty);
    static uint64_t southFill(uint64_t sliders, uint64_t empty);
    static uint64_t eastFill(uint64_t sliders, uint64_t empty);
    static uint64_t westFill(uint64_t sliders, uint64_t empty);
    static uint64_t northEastFill(uint64_t sliders, uint64_t empty);
    static uint64_t northWestFill(uint64_t sliders, uint64_t empty);
    static uint64_t southEastFill(uint64_t sliders, uint64_t empty);
    static uint64_t southWestFill(uint64_t sliders, uint64_t empty);

    static PieceType getPromoType(uint16_t flag);
    static int16_t getDirectionOffset(Directions dir);
    static int16_t getDirectionOffset(int dir);
    static Directions getOppositeDirection(int dir);
    static Directions getOppositeDirection(Directions dir);
    static PieceColor getOppositeColor(PieceColor color) { return static_cast<Board::PieceColor>((color + 1)%2); }
    static bool isNegative(Directions dir);

    static int getFile(uint64_t BB);
    static uint16_t bitScan(uint64_t BB, bool reverse);
    static uint16_t bitScanForward(uint64_t BB) { assert(BB != 0); return std::countr_zero(BB); }
    static uint16_t bitScanReverse(uint64_t BB) { assert(BB != 0); return 63 - std::countl_zero(BB); }
    static uint16_t serializeSingleBit(uint64_t BB) { return bitScanForward(BB); } //get square indices from bitboards
    static uint16_t serializeBitboard(uint64_t BB, std::array<uint16_t, NUM_SQUARES>& indBuf); //serialize into indBuf and return size
    static int materialBalance(Board* board); //positive for white material advantage, negative for black material advantage

    static uint16_t getIndexSquare(std::string square);
    static std::string getIndexStr(uint16_t index);
    static std::string getMoveString(Move move);
    static void printBitBoard(uint64_t BB);
};