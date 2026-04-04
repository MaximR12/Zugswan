#include "Board.h"

uint64_t wpAttackTargets(uint64_t wPawns) {
    uint64_t westAttacks = Board::shiftNorthwest(wPawns);
    uint64_t eastAttacks = Board::shiftNortheast(wPawns); 
    return westAttacks | eastAttacks;
}

uint64_t bpAttackTargets(uint64_t bPawns) {
    uint64_t westAttacks = Board::shiftSouthwest(bPawns);
    uint64_t eastAttacks = Board::shiftSoutheast(bPawns); 
    return westAttacks | eastAttacks;
}

void initPawnAttackTables(std::array<uint64_t, NUM_SQUARES>& whitePawnAttackTable, std::array<uint64_t, NUM_SQUARES>& blackPawnAttackTable) {
    uint64_t currBB = 1;
    for(int i = ROW_LEN; i < NUM_SQUARES-ROW_LEN; ++i, currBB <<= 1) 
        whitePawnAttackTable[i] = wpAttackTargets(currBB);

    currBB = 1;
    for(int i = ROW_LEN; i < NUM_SQUARES-ROW_LEN; ++i, currBB <<= 1) 
        blackPawnAttackTable[i] = bpAttackTargets(currBB);        
}   

uint64_t kingAttackTargets(uint64_t squareSet) {
    uint64_t attacks = Board::shiftEast(squareSet) | Board::shiftWest(squareSet);
    squareSet |= attacks;
    attacks |= (Board::shiftNorth(squareSet) | Board::shiftSouth(squareSet));
    return attacks;
}

void initKingMoveTable(std::array<uint64_t, NUM_SQUARES>& kingAttackTable) {
    uint64_t currBB = 1;
    for(int i = 0; i < NUM_SQUARES; ++i, currBB <<= 1) 
        kingAttackTable[i] = kingAttackTargets(currBB);
}

uint64_t knightAttackTargets(uint64_t squareSet) {
    uint64_t eastOne, eastTwo, westOne, westTwo, northOne, southOne, attacks;
    eastOne = Board::shiftEast(squareSet);
    westOne = Board::shiftWest(squareSet);

    northOne = Board::shiftNorth(eastOne|westOne);
    attacks = Board::shiftNorth(northOne);
    southOne = Board::shiftSouth(eastOne|westOne);
    attacks |= Board::shiftSouth(southOne);

    eastTwo = Board::shiftEast(eastOne);
    westTwo = Board::shiftWest(westOne);
    attacks |= Board::shiftNorth(eastTwo|westTwo);
    attacks |= Board::shiftSouth(eastTwo|westTwo);
    return attacks;
}

void initKnightMoveTable(std::array<uint64_t, NUM_SQUARES>& knightMoveTable) {
    uint64_t currBB = 1;
    for(int sq = 0; sq < NUM_SQUARES; ++sq, currBB <<= 1) 
        knightMoveTable[sq] = knightAttackTargets(currBB);
}

void initRankAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_DIRECTIONS>& rayAttackTable) {
    uint64_t west = 0x00000000000000FEULL, nextRank = RANK_1;
    for(int sq = 0; sq < NUM_SQUARES; ++sq, west <<= 1) {
        if(sq % 8 == 0) nextRank <<= 8;
        rayAttackTable[Board::west][sq] = west&~nextRank;
    }

    nextRank = RANK_8;
    uint64_t east = 0x7F00000000000000ULL;
    for(int sq = NUM_SQUARES-1; sq >= 0; --sq, east >>= 1) {
        if(sq % 8 == 0) nextRank >>= 8;
        rayAttackTable[Board::east][sq] = east&~nextRank;
    }
}

void initFileAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_DIRECTIONS>& rayAttackTable) {
    uint64_t north = 0x0101010101010100ULL;
    for(int sq = 0; sq < NUM_SQUARES; ++sq, north <<= 1) 
        rayAttackTable[Board::north][sq] |= north;

    uint64_t south = 0x0080808080808080ULL;
    for(int sq = NUM_SQUARES-1; sq >= 0; --sq, south >>= 1) 
        rayAttackTable[Board::south][sq] |= south;
}

void initDiagAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_DIRECTIONS>& rayAttackTable) {
    uint64_t northEast = 0x8040201008040200ULL;
    for(int file = 0; file < 8; ++file, northEast = Board::shiftEast(northEast)) {
        uint64_t ne = northEast;
        for(int rank = 0; rank < NUM_SQUARES; rank += 8, ne <<= 8)
            rayAttackTable[Board::northEast][rank+file] = ne;
    }

    uint64_t southWest = 0x0040201008040201ULL;
    for(int file = 7; file >= 0; --file, southWest = Board::shiftWest(southWest)) {
        uint64_t sw = southWest;
        for(int rank = NUM_SQUARES-8; rank >= 0; rank -= 8, sw >>= 8)
            rayAttackTable[Board::southWest][rank+file] = sw;
    }
}

void initAntiDiagAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_DIRECTIONS>& rayAttackTable) {
    uint64_t northWest = 0x0102040810204000ULL;
    for(int file = 7; file >= 0; --file, northWest = Board::shiftWest(northWest)) {
        uint64_t nw = northWest;
        for(int rank = 0; rank < NUM_SQUARES; rank += 8, nw <<= 8)
            rayAttackTable[Board::northWest][rank+file] = nw;
    }

    uint64_t southEast = 0x0002040810204080ULL;
    for(int file = 0; file < 8; ++file, southEast = Board::shiftEast(southEast)) {
        uint64_t se = southEast;
        for(int rank = NUM_SQUARES-8; rank >= 0; rank -= 8, se >>= 8)
            rayAttackTable[Board::northWest][rank+file] = se;
    }
}

void initRayAttackTable(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_DIRECTIONS>& rayAttackTable) 
{
    initRankAttacks(rayAttackTable);
    initFileAttacks(rayAttackTable);
    initDiagAttacks(rayAttackTable);
    initAntiDiagAttacks(rayAttackTable);
}

void Board::initMoveTables() {
    initPawnAttackTables(m_whitePawnAttackTable, m_blackPawnAttackTable);
    initKnightMoveTable(m_knightMoveTable);
    initKingMoveTable(m_kingMoveTable);
    initRayAttackTable(m_rayAttackTable);

    for(int sq = 0; sq < NUM_SQUARES; ++sq) {
        m_bishopMoveTable[sq] = m_rayAttackTable[Board::northWest][sq] | m_rayAttackTable[Board::northEast][sq] 
                                | m_rayAttackTable[Board::southWest][sq] | m_rayAttackTable[Board::southEast][sq];
        m_rookMoveTable[sq] = m_rayAttackTable[Board::north][sq] | m_rayAttackTable[Board::south][sq] 
                                | m_rayAttackTable[Board::east][sq] | m_rayAttackTable[Board::west][sq];
        m_queenMoveTable[sq] = m_bishopMoveTable[sq] | m_rookMoveTable[sq];
    }
}

Board::PieceType Board::getPieceType(uint16_t ind) const {
    assert(ind >= 0 && ind < NUM_SQUARES); 

    PieceType color = this->getPieceColor(ind);
    if(color == whitePieces) {
        for(int curPieceSet = whitePawns; curPieceSet < whiteKing; ++curPieceSet) { //loop over white piece bitboards
            if(m_pieceBB[curPieceSet]&(1ULL<<ind))
                return static_cast<PieceType>(curPieceSet);
        }   
    } else {
        for(int curPieceSet = blackPawns; curPieceSet < blackKing; ++curPieceSet) { //loop over black piece bitboards
            if(m_pieceBB[curPieceSet]&(1ULL<<ind))
                return static_cast<PieceType>(curPieceSet);
        }   
    }

    return invalid;
}

uint16_t Board::serializeBitboard(uint64_t BB, std::array<uint16_t, NUM_SQUARES>& indBuf) {
    uint16_t count = 0;
    while(BB) {
        indBuf[count++] = bitScanForward(BB);
        BB &= (BB - 1);
    }
    return count;
}