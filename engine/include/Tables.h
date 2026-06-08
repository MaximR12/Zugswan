#pragma once

#include <array>
#include "Board.h"

class Tables {
private:
    std::array<std::array<uint64_t, NUM_SQUARES>, 2> m_pawnAttackTable;
    std::array<uint64_t, NUM_SQUARES> m_kingMoveTable;
    std::array<uint64_t, NUM_SQUARES> m_knightMoveTable;

public:
    Tables();

    uint64_t getPawnSquareAttacks(uint16_t ind, Board::PieceColor color) const { assert(ind >= 0 && ind < NUM_SQUARES && color < 2); return m_pawnAttackTable[color][ind]; }
    uint64_t getKingMoves(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_kingMoveTable[ind]; }
    uint64_t getKnightMoves(uint16_t ind) const { assert(ind >= 0 && ind < NUM_SQUARES); return m_knightMoveTable[ind]; }
};