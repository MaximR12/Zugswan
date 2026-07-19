#pragma once

#include <array>
#include "board.hpp"

constexpr int SEED = 1070372;

class Random {
private:
    int seed;

public:
    Random() : seed{SEED} { } 

    //xorshift64star psuedo random generator from stockfish: https://github.com/official-stockfish/Stockfish/blob/master/src/misc.h#L443
    uint64_t getRand() {
        seed ^= seed >> 12, seed ^= seed << 25, seed ^= seed >> 27;
        return seed * 2685821657736338717LL;
    }
};

namespace Tables {
    inline Random rand;
    inline bool initialized = false;
    
    uint64_t getRayMoves(uint16_t ind, Board::Directions dir);
    uint64_t getSliderMoves(uint16_t ind, Board::SliderRays dir);

    uint64_t pawnAttacks(Board::PieceColor side, uint16_t ind);
    uint64_t knightMoves(uint16_t ind);
    uint64_t kingMoves(uint16_t ind);
    uint64_t bishopAttacks(uint16_t square, uint64_t occupied);
    uint64_t rookAttacks(uint16_t square, uint64_t occupied);

    constexpr size_t PIECE_TYPES = 6;
    constexpr size_t FILES = 8;

    inline struct ZobristTable {
        std::array<std::array<uint64_t, NUM_SQUARES>, PIECE_TYPES> pieces;
        std::array<uint64_t, Board::numCastleRights> castleRights;
        std::array<uint64_t, FILES> epFiles;
        uint64_t blackSide; 
    } ZTable;

    void init();
}