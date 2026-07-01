#pragma once

#include <array>
#include "board.hpp"

constexpr size_t MAGIC_TABLE_SIZE = 107648;

namespace Tables {
    inline bool initialized = false;

    inline std::array<std::array<uint64_t, NUM_SQUARES>, 2> pawnAttackTable;
    inline std::array<uint64_t, NUM_SQUARES> kingMoveTable;
    inline std::array<uint64_t, NUM_SQUARES> knightMoveTable;

    uint64_t getRayMoves(uint16_t ind, Board::Directions dir);

    uint64_t getSliderMoves(uint16_t ind, Board::SliderRays dir);

    struct magicEntry {
        size_t offset;
        uint64_t occMask;
        uint64_t magic;
        int shift;
    };

    uint64_t bishopAttacks(uint16_t square, uint64_t occupied);
    uint64_t rookAttacks(uint16_t square, uint64_t occupied);

    void init();
}