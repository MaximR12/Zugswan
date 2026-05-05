#pragma once

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

class Move {
private:
    uint16_t m_move;

public:
    Move() { }
    Move(uint16_t flags, uint16_t from, uint16_t to) { 
        m_move = ((flags&0xF)<<12) | ((to&0x3F)<<6) | (from&0x3F);
    }

    uint16_t getFrom() const { return m_move&0x3F; }
    uint16_t getTo() const { return (m_move>>6)&0x3F; }
    uint16_t getFlag() const { return (m_move>>12)&0x0F; }

    bool isCapture() const { return getFlag()&CAPTURE; }
};