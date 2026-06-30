#include "board.hpp"

#include <unordered_map>
#include <sstream>
#include <format>

uint64_t Board::whitePawnTargets(uint64_t wPawns) {
    uint64_t westAttacks = Board::shift<Board::northWest>(wPawns);
    uint64_t eastAttacks = Board::shift<Board::northEast>(wPawns); 
    return westAttacks | eastAttacks;
}

uint64_t Board::blackPawnTargets(uint64_t bPawns) {
    uint64_t westAttacks = Board::shift<Board::southWest>(bPawns);
    uint64_t eastAttacks = Board::shift<Board::southEast>(bPawns); 
    return westAttacks | eastAttacks;
}

uint64_t Board::kingAttackTargets(uint64_t squareSet) {
    uint64_t attacks = Board::shift<Board::east>(squareSet) | Board::shift<Board::west>(squareSet);
    squareSet |= attacks;
    attacks |= (Board::shift<Board::north>(squareSet) | Board::shift<Board::south>(squareSet));
    return attacks;
}

uint64_t Board::knightAttackTargets(uint64_t squareSet) {
    uint64_t eastOne, eastTwo, westOne, westTwo, northOne, southOne, attacks;
    eastOne = Board::shift<Board::east>(squareSet);
    westOne = Board::shift<Board::west>(squareSet);

    northOne = Board::shift<Board::north>(eastOne|westOne);
    attacks = Board::shift<Board::north>(northOne);
    southOne = Board::shift<Board::south>(eastOne|westOne);
    attacks |= Board::shift<Board::south>(southOne);

    eastTwo = Board::shift<Board::east>(eastOne);
    westTwo = Board::shift<Board::west>(westOne);
    attacks |= Board::shift<Board::north>(eastTwo|westTwo);
    attacks |= Board::shift<Board::south>(eastTwo|westTwo);
    return attacks;
}

consteval std::array<int16_t, NUM_TOTAL_DIRECTIONS> genDirectionOffsetTable() {
    constexpr std::array<int16_t, NUM_TOTAL_DIRECTIONS> directionOffsets {
        northOffset, southOffset, eastOffset, westOffset, northEastOffset, northWestOffset, 
        southWestOffset, southEastOffset, northNorthEastOffset, northEastEastOffset, northNorthWestOffset,
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

uint64_t Board::northFill(uint64_t sliders, uint64_t empty) {
    sliders |= empty & (sliders << 8);
    empty &= empty << 8;
    sliders |= empty & (sliders << 16);
    empty &= empty << 16;
    sliders |= empty & (sliders << 32);
    return shift<Board::north>(sliders);
}

uint64_t Board::southFill(uint64_t sliders, uint64_t empty) {
    sliders |= empty & (sliders >> 8);
    empty &= empty >> 8;
    sliders |= empty & (sliders >> 16);
    empty &= empty >> 16;
    sliders |= empty & (sliders >> 32);
    return shift<Board::south>(sliders);
}

uint64_t Board::eastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders << 1);
    empty &= empty << 1;
    sliders |= empty & (sliders << 2);
    empty &= empty << 2;
    sliders |= empty & (sliders << 4);
    return shift<Board::east>(sliders);
}

uint64_t Board::westFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders >> 1);
    empty &= empty >> 1;
    sliders |= empty & (sliders >> 2);
    empty &= empty >> 2;
    sliders |= empty & (sliders >> 4);
    return shift<Board::west>(sliders);
}

uint64_t Board::northEastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders << 9);
    empty &= empty << 9;
    sliders |= empty & (sliders << 18);
    empty &= empty << 18;
    sliders |= empty & (sliders << 36);
    return shift<Board::northEast>(sliders);
}

uint64_t Board::northWestFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders << 7);
    empty &= empty << 7;
    sliders |= empty & (sliders << 14);
    empty &= empty << 14;
    sliders |= empty & (sliders << 28);
    return shift<Board::northWest>(sliders);
}

uint64_t Board::southEastFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_A_FILE;
    sliders |= empty & (sliders >> 7);
    empty &= empty >> 7;
    sliders |= empty & (sliders >> 14);
    empty &= empty >> 14;
    sliders |= empty & (sliders >> 28);
    return shift<Board::southEast>(sliders);
}

uint64_t Board::southWestFill(uint64_t sliders, uint64_t empty) {
    empty &= NOT_H_FILE;
    sliders |= empty & (sliders >> 9);
    empty &= empty >> 9;
    sliders |= empty & (sliders >> 18);
    empty &= empty >> 18;
    sliders |= empty & (sliders >> 36);
    return shift<Board::southWest>(sliders);
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