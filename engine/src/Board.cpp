#include "board.hpp"

#include <unordered_map>
#include <sstream>
#include <format>

uint64_t Board::wpAttackTargetsEastSafe(uint64_t wPawns, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween) {
    uint64_t pinSafe = wPawns & ~(allInBetween ^ diagInBetween); 
    return Board::shiftNorthEast(pinSafe);
}

uint64_t Board::wpAttackTargetsWestSafe(uint64_t wPawns, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween) {
    uint64_t pinSafe = wPawns & ~(allInBetween ^ antiInBetween);
    return Board::shiftNorthWest(pinSafe);
}

uint64_t Board::bpAttackTargetsEastSafe(uint64_t bPawns, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween) {
    uint64_t pinSafe = bPawns & ~(allInBetween ^ antiInBetween); 
    return Board::shiftSouthEast(pinSafe);
}

uint64_t Board::bpAttackTargetsWestSafe(uint64_t bPawns, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween) {
    uint64_t pinSafe = bPawns & ~(allInBetween ^ diagInBetween); 
    return Board::shiftSouthWest(pinSafe);
}

//maps to correct safe pawn attack func given north or south pawn direction
constinit const std::array<uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t), 4> pawnSafeAttackFuncMap { 
    Board::wpAttackTargetsEastSafe, Board::bpAttackTargetsEastSafe, Board::wpAttackTargetsWestSafe, Board::bpAttackTargetsWestSafe 
};

uint64_t Board::pawnAttackTargetsSafe(uint64_t pawns, PieceColor color, Directions dir, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween) { 
    uint64_t (*func)(uint64_t, uint64_t, uint64_t, uint64_t) = pawnSafeAttackFuncMap[(dir%2)*2 + color];
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

uint64_t Board::pawnAttackTargets(uint64_t pawns, PieceColor color) { 
    uint64_t (*func)(uint64_t) = pawnAttackFuncMap[color];
    return func(pawns);
}

uint64_t Board::kingAttackTargets(uint64_t squareSet) {
    uint64_t attacks = Board::shiftEast(squareSet) | Board::shiftWest(squareSet);
    squareSet |= attacks;
    attacks |= (Board::shiftNorth(squareSet) | Board::shiftSouth(squareSet));
    return attacks;
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

consteval std::array<int16_t, NUM_TOTAL_DIRECTIONS> genDirectionOffsetTable() {
    constexpr std::array<int16_t, NUM_TOTAL_DIRECTIONS> directionOffsets {
        northOffset, southOffset, eastOffset, westOffset, northEastOffset, northWestOffset, 
        southEastOffset, southEastOffset, northNorthEastOffset, northEastEastOffset, northNorthWestOffset,
        northWestWestOffset, southSouthEastOffset, southEastEastOffset, southSouthWestOffset, southWestWestOffset
    };

    return directionOffsets;
}

consteval std::array<Board::Directions, NUM_TOTAL_DIRECTIONS> genOppositeDirectionTable() {
    constexpr std::array<Board::Directions, NUM_TOTAL_DIRECTIONS> oppositeDirections {
        Board::south, Board::north, Board::west, Board::east, Board::southEast, Board::southWest,
        Board::northEast, Board::northWest, Board::southSouthWest, Board::southWestWest, Board::southSouthEast,
        Board::southEastEast, Board::northNorthWest, Board::northWestWest, Board::northNorthEast, Board::northEastEast,
    };

    return oppositeDirections;
}

constinit const std::array<int16_t, NUM_TOTAL_DIRECTIONS> directionOffsetTable { genDirectionOffsetTable() }; 
constinit const std::array<Board::Directions, NUM_TOTAL_DIRECTIONS> oppositeDirectionTable { genOppositeDirectionTable() }; 

int16_t Board::getDirectionOffset(Directions dir) { 
    assert(dir >= 0 && dir <= NUM_TOTAL_DIRECTIONS);
    return directionOffsetTable[dir];
}

int16_t Board::getDirectionOffset(int dir) { 
    assert(dir >= 0 && dir <= NUM_TOTAL_DIRECTIONS);
    return directionOffsetTable[dir];
}

Board::Directions Board::getOppositeDirection(Directions dir) {
    assert(dir >= 0 && dir <= NUM_TOTAL_DIRECTIONS);
    return oppositeDirectionTable[dir];
}

Board::Directions Board::getOppositeDirection(int dir) {
    assert(dir >= 0 && dir <= NUM_TOTAL_DIRECTIONS);
    return oppositeDirectionTable[dir];
}

bool Board::isNegative(Directions dir) {
    return directionOffsetTable[dir] < 0;
}

constinit const std::array<Board::Directions, 2> pawnDirTable { Board::north, Board::south };

Board::Directions Board::getPawnDirection(PieceColor color) {
    return pawnDirTable[color];
}

uint64_t Board::pawnShift(uint64_t BB, Directions dir) {
    uint64_t negDirMask = 0ULL - static_cast<uint64_t>(isNegative(dir));
    return (BB << directionOffsetTable[dir] & ~negDirMask) | (BB >> -directionOffsetTable[dir] & negDirMask);
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

uint64_t Board::northFill(uint64_t sliders, uint64_t empty) {
    sliders |= empty & (sliders << 8);
    empty &= empty << 8;
    sliders |= empty & (sliders << 16);
    empty &= empty << 16;
    sliders |= empty & (sliders << 32);
    return shiftNorth(sliders);
}

uint64_t Board::southFill(uint64_t sliders, uint64_t empty) {
    sliders |= empty & (sliders >> 8);
    empty &= empty >> 8;
    sliders |= empty & (sliders >> 16);
    empty &= empty >> 16;
    sliders |= empty & (sliders >> 32);
    return shiftSouth(sliders);
}

uint64_t Board::eastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders << 1);
    empty &= empty << 1;
    sliders |= empty & (sliders << 2);
    empty &= empty << 2;
    sliders |= empty & (sliders << 4);
    return shiftEast(sliders);
}

uint64_t Board::westFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders >> 1);
    empty &= empty >> 1;
    sliders |= empty & (sliders >> 2);
    empty &= empty >> 2;
    sliders |= empty & (sliders >> 4);
    return shiftWest(sliders);
}

uint64_t Board::northEastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders << 9);
    empty &= empty << 9;
    sliders |= empty & (sliders << 18);
    empty &= empty << 18;
    sliders |= empty & (sliders << 36);
    return shiftNorthEast(sliders);
}

uint64_t Board::northWestFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders << 7);
    empty &= empty << 7;
    sliders |= empty & (sliders << 14);
    empty &= empty << 14;
    sliders |= empty & (sliders << 28);
    return shiftNorthWest(sliders);
}

uint64_t Board::southEastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders >> 7);
    empty &= empty >> 7;
    sliders |= empty & (sliders >> 14);
    empty &= empty >> 14;
    sliders |= empty & (sliders >> 28);
    return shiftSouthEast(sliders);
}

uint64_t Board::southWestFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders >> 9);
    empty &= empty >> 9;
    sliders |= empty & (sliders >> 18);
    empty &= empty >> 18;
    sliders |= empty & (sliders >> 36);
    return shiftSouthWest(sliders);
}

Board::PieceType Board::getPieceType(uint16_t ind) const {
    assert(ind >= 0 && ind < NUM_SQUARES); 
    PieceColor color = this->getPieceColor(ind);
    for(int curPieceSet = pawns; curPieceSet <= king; ++curPieceSet) { //loop over all piece types
        if(m_pieceBB[color][curPieceSet]&(1ULL<<ind))
            return static_cast<PieceType>(curPieceSet);
    }   
    return invalid;
}

uint16_t Board::bitScan(uint64_t BB, bool reverse) {
    assert(BB != 0);
    uint64_t rMask = 0ULL - static_cast<uint64_t>(reverse);
    BB &= (0ULL-BB) | rMask;
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

int materialCount(Board* board, Board::PieceColor side) {
    std::unordered_map<int, int> valueMap {
        {Board::pawns, 1}, {Board::knights, 3}, {Board::bishops, 3}, {Board::rooks, 5}, {Board::queens, 9}
    };

    int material = 0;
    for(int type = 0; type < 6; ++type) {
        int count = std::popcount(board->getPieceSet(static_cast<Board::PieceType>(type), Board::white));
        material += count * valueMap[type];
    }

    return material;
}

int Board::materialBalance(Board* board) {
    return materialCount(board, white) - materialCount(board, black);
}

void Board::clearPosition() {
    for(int color = 0; color < 2; ++color)
        for(int type = 0; type < NUM_PIECE_TYPES; ++type)
            updateBB(static_cast<PieceType>(type), static_cast<PieceColor>(color), EMPTY);

    updateOccupiedBB(EMPTY);
    updateEmptyBB(UNIVERSE);
}

Board::PieceColor Board::loadPosition(std::string& fen) {
    std::istringstream is(fen);
    std::string pos, side, castleRights, epTarget, halfMoveClock, fullMoveCounter;
    is >> pos >> side >> castleRights >> epTarget >> halfMoveClock >> fullMoveCounter;
    PieceColor turn = side == "w" ? white : black;

    std::unordered_map<char, PieceType> typeMap {
        {'P', pawns}, {'N', knights}, {'B', bishops}, {'R', rooks}, {'Q', queens}, {'K', king}
    };

    clearPosition();
    uint16_t rank = 7, file = 0, index;
    for(char c : pos) {
        if(c == '/') {
            --rank, file = 0;    
            continue;
        }
        if(c < 65) {
            file += (c - '0');
            continue;
        }

        PieceColor color = c > 90 ? black : white;
        if(c > 90) c -= 32;
        PieceType type = typeMap[c];

        index = rank * 8 + file; 
        updateBB(type, color, getPieceSet(type, color) | (1ULL << index));
        updateBB(all, color, getPieceSet(all, color) | (1ULL << index));

        ++file;
    }

    updateOccupiedBB(getPieceSet(all, black) | getPieceSet(all, white));
    updateEmptyBB(~getOccupied());

    m_kingCastleRights = {false, false}, m_queenCastleRights = {false, false};
    for(char c : castleRights) {
        if(c == '-') 
            break;

        PieceColor color = c > 90 ? black : white;
        if(c > 90) c -= 32;

        if(typeMap[c] == king) m_kingCastleRights[color] = true;
        else m_queenCastleRights[color] = true;
    }

    m_enPassantTargets[white] = EMPTY, m_enPassantTargets[black] = EMPTY;
    if(epTarget != "-") {
        int file = epTarget[0] - 96, rank = epTarget[1] - '0';
        uint16_t index = 8 * (rank-1) + file-1;
        
        updateEnPassantTargets(turn, 1ULL<<index);
    }

    m_halfMoveClock = std::stoi(halfMoveClock);
    m_fullMoveCounter = std::stoi(fullMoveCounter);

    return turn;
}

uint16_t Board::getIndexSquare(std::string square) {
    assert(square.size() == 2);
    char first = square[0], second = square[1];
    uint16_t rank = second - '0';
    uint16_t file = first - 'a';
    return (rank-1) * 8 + file;
}

std::string Board::getIndexStr(uint16_t index) {
    assert(index < NUM_SQUARES);
    char rank = (index / 8 + 1) + '0';
    char file = index % 8 + 'a';
    return std::string{file} + std::string{rank}; 
}

std::string Board::getMoveString(Move move) {
    uint16_t from = move.getFrom(), to = move.getTo();
    return std::format("{}{}", getIndexStr(from), getIndexStr(to));
}

void Board::printBitBoard(uint64_t BB) {
    std::bitset<64> bb{BB};
    for(int i = 63; i >= 0; --i) {
        std::cout << bb[i]; 
        if(i % 8 == 0) std::cout << '\n';
    }
    std::cout << '\n';
}