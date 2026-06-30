#include "tables.hpp"
#include <chrono>

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
    for(int file = 0; file < 8; ++file, northEast = Board::shift<Board::east>(northEast)) {
        uint64_t ne = northEast;
        for(int rank = 0; rank < NUM_SQUARES; rank += 8, ne <<= 8)
            rayAttackTable[Board::northEast][rank+file] = ne;
    }

    uint64_t southWest = 0x0040201008040201ULL;
    for(int file = 7; file >= 0; --file, southWest = Board::shift<Board::west>(southWest)) {
        uint64_t sw = southWest;
        for(int rank = NUM_SQUARES-8; rank >= 0; rank -= 8, sw >>= 8)
            rayAttackTable[Board::southWest][rank+file] = sw;
    }
}

void initAntiDiagAttacks(std::array<std::array<uint64_t, NUM_SQUARES>, NUM_SLIDER_DIRECTIONS>& rayAttackTable) {
    uint64_t northWest = 0x0102040810204000ULL;
    for(int file = 7; file >= 0; --file, northWest = Board::shift<Board::west>(northWest)) {
        uint64_t nw = northWest;
        for(int rank = 0; rank < NUM_SQUARES; rank += 8, nw <<= 8)
            rayAttackTable[Board::northWest][rank+file] = nw;
    }

    uint64_t southEast = 0x0002040810204080ULL;
    for(int file = 0; file < 8; ++file, southEast = Board::shift<Board::east>(southEast)) {
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

constexpr uint64_t Tables::getRayMoves(uint16_t ind, Board::Directions dir) {
    assert(ind >= 0 && ind < NUM_SQUARES);
    return rayAttackTable[dir][ind]; 
} 

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

using namespace Tables;
//Magic number values, taken from Aurora-Chess-Engine: https://github.com/kjljixx/Aurora-Chess-Engine/blob/main/lookup.h
const std::array<uint64_t, 64> rookMagics = {
  0xa8002c000108020ULL, 0x6c00049b0002001ULL, 0x100200010090040ULL, 0x2480041000800801ULL, 0x280028004000800ULL,
  0x900410008040022ULL, 0x280020001001080ULL, 0x2880002041000080ULL, 0xa000800080400034ULL, 0x4808020004000ULL,
  0x2290802004801000ULL, 0x411000d00100020ULL, 0x402800800040080ULL, 0xb000401004208ULL, 0x2409000100040200ULL,
  0x1002100004082ULL, 0x22878001e24000ULL, 0x1090810021004010ULL, 0x801030040200012ULL, 0x500808008001000ULL,
  0xa08018014000880ULL, 0x8000808004000200ULL, 0x201008080010200ULL, 0x801020000441091ULL, 0x800080204005ULL,
  0x1040200040100048ULL, 0x120200402082ULL, 0xd14880480100080ULL, 0x12040280080080ULL, 0x100040080020080ULL,
  0x9020010080800200ULL, 0x813241200148449ULL, 0x491604001800080ULL, 0x100401000402001ULL, 0x4820010021001040ULL,
  0x400402202000812ULL, 0x209009005000802ULL, 0x810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL,
  0x40204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 0x804040008008080ULL,
  0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 0x280009023410300ULL, 0xe0100040002240ULL,
  0x200100401700ULL, 0x2244100408008080ULL, 0x8000400801980ULL, 0x2000810040200ULL, 0x8010100228810400ULL,
  0x2000009044210200ULL, 0x4080008040102101ULL, 0x40002080411d01ULL, 0x2005524060000901ULL, 0x502001008400422ULL,
  0x489a000810200402ULL, 0x1004400080a13ULL, 0x4000011008020084ULL, 0x26002114058042ULL
};
const std::array<uint64_t, 64> bishopMagics = {
  0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 0x4042004000000ULL,
  0x100822020200011ULL, 0xc00444222012000aULL, 0x28808801216001ULL, 0x400492088408100ULL, 0x201c401040c0084ULL,
  0x840800910a0010ULL, 0x82080240060ULL, 0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL,
  0x8144042209100900ULL, 0x208081020014400ULL, 0x4800201208ca00ULL, 0xf18140408012008ULL, 0x1004002802102001ULL,
  0x841000820080811ULL, 0x40200200a42008ULL, 0x800054042000ULL, 0x88010400410c9000ULL, 0x520040470104290ULL,
  0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL, 0x941408200c002000ULL, 0x658810000806011ULL,
  0x188071040440a00ULL, 0x4800404002011c00ULL, 0x104442040404200ULL, 0x511080202091021ULL, 0x4022401120400ULL,
  0x80c0040400080120ULL, 0x8040010040820802ULL, 0x480810700020090ULL, 0x102008e00040242ULL, 0x809005202050100ULL,
  0x8002024220104080ULL, 0x431008804142000ULL, 0x19001802081400ULL, 0x200014208040080ULL, 0x3308082008200100ULL,
  0x41010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL, 0x111040120082000ULL, 0x6803040141280a00ULL,
  0x2101004202410000ULL, 0x8200000041108022ULL, 0x21082088000ULL, 0x2410204010040ULL, 0x40100400809000ULL,
  0x822088220820214ULL, 0x40808090012004ULL, 0x910224040218c9ULL, 0x402814422015008ULL, 0x90014004842410ULL,
  0x1000042304105ULL, 0x10008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL
};

const std::array<int, 64> rookIndexBits = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

const std::array<int, 64> bishopIndexBits = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

uint64_t bishopOccMask(int ind) {
    uint64_t mask = (Tables::getRayMoves(ind, Board::northEast) & NOT_LAST_RANK & NOT_H_FILE);
    mask |= (Tables::getRayMoves(ind, Board::southEast) & NOT_LAST_RANK & NOT_H_FILE);
    mask |= (Tables::getRayMoves(ind, Board::northWest) & NOT_LAST_RANK & NOT_A_FILE);
    mask |= (Tables::getRayMoves(ind, Board::southWest) & NOT_LAST_RANK & NOT_A_FILE);

    return mask;
}

uint64_t rookOccMask(int ind) {
    uint64_t mask = (Tables::getRayMoves(ind, Board::north) & NOT_LAST_RANK);
    mask |= (Tables::getRayMoves(ind, Board::south) & NOT_LAST_RANK);
    mask |= (Tables::getRayMoves(ind, Board::east) & NOT_H_FILE);
    mask |= (Tables::getRayMoves(ind, Board::west) & NOT_A_FILE);

    return mask;
}

size_t offset = 0;
std::array<magicEntry, 64> bishopMagic;
std::array<magicEntry, 64> rookMagic;

void initBMagic() {
    for(int i = 0; i < 64; ++i) {
        bishopMagic[i] = magicEntry{offset, bishopOccMask(i), bishopMagics[i], 64 - bishopIndexBits[i]};
        offset += 1ULL << bishopIndexBits[i];
    }
}

void initRMagic() {
    for(int i = 0; i < 64; ++i) {
        rookMagic[i] = magicEntry{offset, rookOccMask(i), rookMagics[i], 64 - rookIndexBits[i]};
        offset += 1ULL << rookIndexBits[i];
    }
}

std::array<uint64_t, MAGIC_TABLE_SIZE> magicTable;

uint64_t bishopMoves(int ind, uint64_t blockers) {
    uint64_t empty = ~blockers, slider = 1ULL<<ind;
    uint64_t moveSet = Board::northEastFill(slider, empty);
    moveSet |= Board::southEastFill(slider, empty);
    moveSet |= Board::northWestFill(slider, empty);
    moveSet |= Board::southWestFill(slider, empty);

    return moveSet;
}

uint64_t rookMoves(int ind, uint64_t blockers) {
    uint64_t empty = ~blockers, slider = 1ULL<<ind;
    uint64_t moveSet = Board::northFill(slider, empty);
    moveSet |= Board::southFill(slider, empty);
    moveSet |= Board::eastFill(slider, empty);
    moveSet |= Board::westFill(slider, empty);

    return moveSet;
}

enum class SliderType {
    bishop, rook
};

template<SliderType slider>
void genMagicTable() {
    if constexpr (slider == SliderType::bishop)
        initBMagic();
    else
        initRMagic();

    std::array<magicEntry, 64> *magicArr;
    if constexpr (slider == SliderType::bishop)
        magicArr = &bishopMagic;
    else
        magicArr = &rookMagic;

    std::array<magicEntry, 64>& magicRef = *magicArr;
    for(int i = 0; i < 64; ++i) {
        size_t offset = magicRef[i].offset;
        uint64_t occMask = magicRef[i].occMask;
        uint64_t magic = magicRef[i].magic;
        uint64_t shift = magicRef[i].shift;
    
        uint64_t blockers = 0ULL;
        do { //iterate all blocker combinations
            if constexpr (slider == SliderType::bishop)
                magicTable[offset + ((blockers * magic) >> shift)] = bishopMoves(i, blockers);
            else
                magicTable[offset + ((blockers * magic) >> shift)] = rookMoves(i, blockers);
            blockers = (blockers - occMask) & occMask;
        } while(blockers);
    }
}

void initMagicTable() {
    genMagicTable<SliderType::bishop>();
    genMagicTable<SliderType::rook>();
}

uint64_t Tables::bishopAttacks(uint16_t square, uint64_t occupied) {
    size_t offset = bishopMagic[square].offset;
    occupied &= bishopMagic[square].occMask;
    occupied *= bishopMagic[square].magic;
    occupied >>= bishopMagic[square].shift;

    return magicTable[offset + occupied];
}

uint64_t Tables::rookAttacks(uint16_t square, uint64_t occupied) {
    size_t offset = rookMagic[square].offset;
    occupied &= rookMagic[square].occMask;
    occupied *= rookMagic[square].magic;
    occupied >>= rookMagic[square].shift;

    return magicTable[offset + occupied];
}

void Tables::init() {
    initPawnAttackTables(pawnAttackTable);
    initKnightMoveTable(knightMoveTable);
    initKingMoveTable(kingMoveTable);
    initMagicTable();

    initialized = true;
}