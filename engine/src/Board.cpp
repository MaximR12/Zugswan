#include "Board.h"

uint64_t Board::wpAttackTargetsSafe(uint64_t wPawns, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween) {
    uint64_t pinSafe = wPawns & ~(allInBetween ^ diagInBetween);
    uint64_t westAttacks = Board::shiftNorthWest(pinSafe);
    pinSafe = wPawns & ~(allInBetween ^ antiInBetween);
    uint64_t eastAttacks = Board::shiftNorthEast(pinSafe); 
    return westAttacks | eastAttacks;
}

uint64_t Board::bpAttackTargetsSafe(uint64_t bPawns, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween) {
    uint64_t pinSafe = bPawns & ~(allInBetween ^ antiInBetween);
    uint64_t westAttacks = Board::shiftSouthWest(pinSafe);
    pinSafe = bPawns & ~(allInBetween ^ diagInBetween);
    uint64_t eastAttacks = Board::shiftSouthEast(pinSafe); 
    return westAttacks | eastAttacks;
}

//maps to correct safe pawn attack func given north or south pawn direction
constinit const std::array<uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t), 2> pawnSafeAttackFuncMap { Board::wpAttackTargetsSafe, Board::bpAttackTargetsSafe };

uint64_t Board::pawnAttackTargetsSafe(uint64_t pawns, Directions pawnDir, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween) { 
    uint64_t (*func)(uint64_t, uint64_t, uint64_t, uint64_t) = pawnSafeAttackFuncMap[pawnDir];
    return func(pawns, diagInBetween, antiInBetween, allInBetween);
}

uint64_t Board::whitePawnTargets(uint64_t wPawns) {
    uint64_t westAttacks = Board::shiftNorthWest(wPawns);
    uint64_t eastAttacks = Board::shiftNorthEast(wPawns); 
    return westAttacks | eastAttacks;
}

uint64_t Board::blackPawnTargets(uint64_t bPawns) {
    uint64_t westAttacks = Board::shiftSouthWest(bPawns);
    uint64_t eastAttacks = Board::shiftSouthEast(bPawns); 
    return westAttacks | eastAttacks;
}

//maps to correct pawn attack func given north or south pawn direction
constinit const std::array<uint64_t (*)(uint64_t), 2> pawnAttackFuncMap { Board::whitePawnTargets, Board::blackPawnTargets };

uint64_t Board::pawnAttackTargets(uint64_t pawns, Directions pawnDir) { 
    uint64_t (*func)(uint64_t) = pawnAttackFuncMap[pawnDir];
    return func(pawns);
}

void initPawnAttackTables(std::array<std::array<uint64_t, NUM_SQUARES>, 2>& pawnAttackTable) {
    uint64_t currBB = 1;
    for(int i = 0; i < NUM_SQUARES; ++i, currBB <<= 1) 
        pawnAttackTable[Board::whitePieces][i] = Board::whitePawnTargets(currBB);

    currBB = 1;
    for(int i = 0; i < NUM_SQUARES; ++i, currBB <<= 1) 
        pawnAttackTable[Board::blackPieces][i] = Board::blackPawnTargets(currBB);        
}   

uint64_t Board::kingAttackTargets(uint64_t squareSet) {
    uint64_t attacks = Board::shiftEast(squareSet) | Board::shiftWest(squareSet);
    squareSet |= attacks;
    attacks |= (Board::shiftNorth(squareSet) | Board::shiftSouth(squareSet));
    return attacks;
}

void initKingMoveTable(std::array<uint64_t, NUM_SQUARES>& kingAttackTable) {
    uint64_t currBB = 1;
    for(int i = 0; i < NUM_SQUARES; ++i, currBB <<= 1) 
        kingAttackTable[i] = Board::kingAttackTargets(currBB);
}

uint64_t Board::knightAttackTargets(uint64_t squareSet) {
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
        knightMoveTable[sq] = Board::knightAttackTargets(currBB);
}

void initRankAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_SLIDER_DIRECTIONS>& rayAttackTable) {
    uint64_t east = 0x00000000000000FEULL, nextRank = RANK_1;
    for(int sq = 0; sq < NUM_SQUARES; ++sq, east <<= 1) {
        if(sq % 8 == 0) nextRank <<= 8;
        rayAttackTable[Board::east][sq] = east&~nextRank;
    }

    nextRank = RANK_7;
    uint64_t west = 0x7F00000000000000ULL;
    for(int sq = NUM_SQUARES-1; sq >= 0; --sq, west >>= 1) {
        rayAttackTable[Board::west][sq] = west&~nextRank;
        if(sq % 8 == 0) nextRank >>= 8;
    }
}

void initFileAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_SLIDER_DIRECTIONS>& rayAttackTable) {
    uint64_t north = 0x0101010101010100ULL;
    for(int sq = 0; sq < NUM_SQUARES; ++sq, north <<= 1) { 
        rayAttackTable[Board::north][sq] = north;
    }

    uint64_t south = 0x0080808080808080ULL;
    for(int sq = NUM_SQUARES-1; sq >= 0; --sq, south >>= 1) 
        rayAttackTable[Board::south][sq] = south;
}

void initDiagAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_SLIDER_DIRECTIONS>& rayAttackTable) {
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

void initAntiDiagAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_SLIDER_DIRECTIONS>& rayAttackTable) {
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
            rayAttackTable[Board::southEast][rank+file] = se;
    }
}

auto initRayAttackTable() 
{
    std::array<std::array<uint64_t, NUM_SQUARES>, NUM_SLIDER_DIRECTIONS> rayAttackTable;
    initRankAttacks(rayAttackTable);
    initFileAttacks(rayAttackTable);
    initDiagAttacks(rayAttackTable);
    initAntiDiagAttacks(rayAttackTable);

    return rayAttackTable;
}

const std::array<std::array<uint64_t, NUM_SQUARES>, NUM_SLIDER_DIRECTIONS> rayAttackTable {initRayAttackTable()};

uint64_t Board::getRayMoves(uint16_t ind, Directions dir) {
    assert(ind >= 0 && ind < NUM_SQUARES);
    return rayAttackTable[dir][ind]; 
} 

Board::Board() {
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
    m_whiteEpTargets = 0ULL;
    m_blackEpTargets = 0ULL;

    initPawnAttackTables(m_pawnAttackTable);
    initKnightMoveTable(m_knightMoveTable);
    initKingMoveTable(m_kingMoveTable);
}

uint64_t Board::northFill(uint64_t sliders, uint64_t empty) {
    sliders |= empty & (sliders << 8);
    empty &= empty << 8;
    sliders |= empty & (sliders << 16);
    empty &= empty << 16;
    sliders |= empty & (sliders << 32);
    return Board::shiftNorth(sliders);
}

uint64_t Board::southFill(uint64_t sliders, uint64_t empty) {
    sliders |= empty & (sliders >> 8);
    empty &= empty >> 8;
    sliders |= empty & (sliders >> 16);
    empty &= empty >> 16;
    sliders |= empty & (sliders >> 32);
    return Board::shiftSouth(sliders);
}

uint64_t Board::eastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders << 1);
    empty &= empty << 1;
    sliders |= empty & (sliders << 2);
    empty &= empty << 2;
    sliders |= empty & (sliders << 4);
    return Board::shiftEast(sliders);
}

uint64_t Board::westFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders >> 1);
    empty &= empty >> 1;
    sliders |= empty & (sliders >> 2);
    empty &= empty >> 2;
    sliders |= empty & (sliders >> 4);
    return Board::shiftWest(sliders);
}

uint64_t Board::northEastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders << 9);
    empty &= empty << 9;
    sliders |= empty & (sliders << 18);
    empty &= empty << 18;
    sliders |= empty & (sliders << 36);
    return Board::shiftNorthEast(sliders);
}

uint64_t Board::northWestFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders << 7);
    empty &= empty << 7;
    sliders |= empty & (sliders << 14);
    empty &= empty << 14;
    sliders |= empty & (sliders << 28);
    return Board::shiftNorthWest(sliders);
}

uint64_t Board::southEastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders >> 7);
    empty &= empty >> 7;
    sliders |= empty & (sliders >> 14);
    empty &= empty >> 14;
    sliders |= empty & (sliders >> 28);
    return Board::shiftSouthEast(sliders);
}

uint64_t Board::southWestFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders >> 9);
    empty &= empty >> 9;
    sliders |= empty & (sliders >> 18);
    empty &= empty >> 18;
    sliders |= empty & (sliders >> 36);
    return Board::shiftSouthWest(sliders);
}

Board::PieceType Board::getPieceType(uint16_t ind) const {
    assert(ind >= 0 && ind < NUM_SQUARES); 

    PieceType color = this->getPieceColor(ind);
    if(color == whitePieces) {
        for(int curPieceSet = whitePawns; curPieceSet <= whiteKing; ++curPieceSet) { //loop over white piece bitboards
            if(m_pieceBB[curPieceSet]&(1ULL<<ind))
                return static_cast<PieceType>(curPieceSet);
        }   
    } else {
        for(int curPieceSet = blackPawns; curPieceSet <= blackKing; ++curPieceSet) { //loop over black piece bitboards
            if(m_pieceBB[curPieceSet]&(1ULL<<ind))
                return static_cast<PieceType>(curPieceSet);
        }   
    }

    return invalid;
}

uint16_t Board::bitScan(uint64_t BB, bool reverse) {
    assert(BB != 0);
    uint64_t rMask = -(uint64_t)reverse;
    BB &= -BB | rMask;
    return bitScanReverse(BB);
}

uint16_t Board::serializeBitboard(uint64_t BB, std::array<uint16_t, NUM_SQUARES>& indBuf) {
    uint16_t count = 0;
    while(BB) {
        indBuf[count++] = bitScanForward(BB);
        BB &= (BB - 1);
    }
    return count;
}

void Board::printBitBoard(uint64_t BB) {
    std::bitset<64> bb{BB};
    for(int i = 63; i >= 0; --i) {
        std::cout << bb[i]; 
        if(i % 8 == 0) std::cout << '\n';
    }
    std::cout << '\n';
}