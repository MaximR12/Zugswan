#include "Tables.h"

void initPawnAttackTables(std::array<std::array<uint64_t, NUM_SQUARES>, 2>& pawnAttackTable) {
    uint64_t currBB = 1;
    for(int i = 0; i < NUM_SQUARES; ++i, currBB <<= 1) 
        pawnAttackTable[Board::white][i] = Board::whitePawnTargets(currBB);

    currBB = 1;
    for(int i = 0; i < NUM_SQUARES; ++i, currBB <<= 1) 
        pawnAttackTable[Board::black][i] = Board::blackPawnTargets(currBB);        
}

void initKnightMoveTable(std::array<uint64_t, NUM_SQUARES>& knightMoveTable) {
    uint64_t currBB = 1;
    for(int sq = 0; sq < NUM_SQUARES; ++sq, currBB <<= 1) 
        knightMoveTable[sq] = Board::knightAttackTargets(currBB);
}

void initKingMoveTable(std::array<uint64_t, NUM_SQUARES>& kingAttackTable) {
    uint64_t currBB = 1;
    for(int i = 0; i < NUM_SQUARES; ++i, currBB <<= 1) 
        kingAttackTable[i] = Board::kingAttackTargets(currBB);
}

Tables::Tables() {
    initPawnAttackTables(m_pawnAttackTable);
    initKnightMoveTable(m_knightMoveTable);
    initKingMoveTable(m_kingMoveTable);
}