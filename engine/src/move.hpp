#pragma once

#include <cassert>
#include <array>
#include <limits>
#include "stdint.h"

/*
Move info encoded in a 16 bit unsigned int
bits 0-5: from square
bits 6-11: to square
bits 12-15: special move flag  
*/

//four bit move flags
constexpr uint16_t QUIET_MOVE = 0x0;
constexpr uint16_t DOUBLE_PAWN_PUSH = 0x1;
constexpr uint16_t KING_CASTLE = 0x2;
constexpr uint16_t QUEEN_CASTLE = 0x3;
constexpr uint16_t CAPTURE = 0x4;
constexpr uint16_t EP_CAPTURE = 0x5;
constexpr uint16_t KNIGHT_PROMOTION = 0x8;
constexpr uint16_t BISHOP_PROMOTION = 0x9;
constexpr uint16_t ROOK_PROMOTION = 0xA;
constexpr uint16_t QUEEN_PROMOTION = 0xB;
constexpr uint16_t KNIGHT_PROMO_CAPTURE = 0xC;
constexpr uint16_t BISHOP_PROMO_CAPTURE = 0xD;
constexpr uint16_t ROOK_PROMO_CAPTURE = 0xE;
constexpr uint16_t QUEEN_PROMO_CAPTURE = 0xF;

//move ordering levels
constexpr int16_t GOOD_CAPTURE_BASE = 30'000;
constexpr int16_t KILLER_BASE = 20'000;
constexpr int16_t QUIET_BASE = 0;
constexpr int16_t BAD_CAPTURE_BASE = -30'000;

constexpr int MAX_LEGAL_MOVES = 256;

class Move {
private:
    uint16_t m_move;

public:
    int16_t score;

    Move() = default;
    Move(uint16_t move) : m_move{move} { }
    Move(uint16_t flag, uint16_t from, uint16_t to) { 
        m_move = ((flag&0xF)<<12) | ((to&0x3F)<<6) | (from&0x3F);
    }

    bool operator==(Move other) { return m_move == other.m_move; }

    uint16_t getFrom() const { return m_move&0x3F; }
    uint16_t getTo() const { return (m_move>>6)&0x3F; }
    uint16_t getFlag() const { return (m_move>>12)&0x0F; }

    bool isCapture() const { return getFlag()&CAPTURE; }
    bool isPromotion() const { return getFlag()&0x8; }
    bool isGoodCapture() const { return score >= GOOD_CAPTURE_BASE; }

    static bool isCapture(uint16_t flag) { return flag&CAPTURE; }
    static bool isPromotion(uint16_t flag) { return flag&0x8; }
    static Move invalid() { return Move(0); }
};

class MoveList {
private:
    std::array<Move, MAX_LEGAL_MOVES> m_data;
    size_t m_size;

public:
    MoveList() : m_size{0} { } 

    size_t size() const { return m_size; }
    Move& back() { return m_data[m_size-1]; }
    Move* begin() { return m_data.data(); }
    Move* end() { return &m_data[m_size]; }
    bool compare(size_t i, Move other) const { return m_data[i] == other; }

    void clear() { m_size = 0; }
    void reorder(const Move elem);
    void push_list(MoveList& vec, size_t pos);
    void push_back() { assert(m_size < MAX_LEGAL_MOVES); m_size++; }
    void push_back(const Move elem) { assert(m_size < MAX_LEGAL_MOVES); m_data[m_size++] = elem; }
    void pop_back() { assert(m_size > 0); --m_size; }
    Move pop_move(const Move elem); //remove and return elem, return first element if not contained
    Move pick_move(); //remove and return highest scoring move
    Move& operator[](size_t i) { return m_data[i]; }
};

inline void MoveList::push_list(MoveList& list, size_t pos) {
    for(size_t i = pos; i < pos + list.size(); ++i)
        m_data[i] = list[i-pos];
    m_size = std::max(pos + list.size(), m_size);
}

inline void MoveList::reorder(const Move elem) {
    for(size_t i = 0; i < m_size; ++i) {
        if(m_data[i] == elem) {
            Move temp = m_data[0];
            m_data[0] = elem;
            m_data[i] = temp;
            return;
        }
    }
}

inline Move MoveList::pick_move() {
    if(m_size == 0)
        return Move::invalid();

    size_t bestInd = 0;
    int16_t maxScore = m_data[0].score;
    for(size_t i = 1; i < m_size; ++i) {
        int16_t currScore = m_data[i].score;
        if(currScore > maxScore)
            maxScore = currScore, bestInd = i;
    }

    Move best = m_data[bestInd];
    m_data[bestInd] = m_data[--m_size];
    
    return best;
}

inline Move MoveList::pop_move(const Move elem) {
    assert(m_size > 0);
    for(size_t i = 0; i < m_size; ++i) {
        if(m_data[i] == elem) {
            m_data[i] = m_data[--m_size];
            return elem;
        }
    }

    --m_size;
    return m_data[0];
}