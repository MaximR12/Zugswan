#include "MoveGen.h"
#include <array>

void appendFromBB(std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint16_t from, uint64_t targets, uint16_t flag) {
   uint16_t numSquares = Board::serializeBitboard(targets, indBuf);
   for(uint16_t sq = 0; sq < numSquares; ++sq) {
      uint16_t toInd = indBuf[sq]; 
      moveList.emplace_back(flag, from, toInd);
   }
}

uint64_t wpSinglePushTargets(Board* const board, uint64_t wPawns, uint64_t empty) {
   return board->shiftNorth(wPawns) & empty;
}

uint64_t wpDoublePushTargets(Board* const board, uint64_t wPawns, uint64_t empty) {
   uint64_t singlePushes = wpSinglePushTargets(board, wPawns, empty);
   return board->shiftNorth(singlePushes) & empty & RANK_4;
}

uint64_t bpSinglePushTargets(Board* const board, uint64_t bPawns, uint64_t empty) {
   return board->shiftSouth(bPawns) & empty;
}

uint64_t bpDoublePushTargets(Board* const board, uint64_t bPawns, uint64_t empty) {
   uint64_t singlePushes = bpSinglePushTargets(board, bPawns, empty);
   return board->shiftSouth(singlePushes) & empty & RANK_5;
}

void appendWhitePawnPushMoves(Board* board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint64_t wPawns, uint64_t bPawns, uint64_t empty) {
   const int singleRankOffset = 8, doubleRankOffset = 16;

   uint16_t numSquares = Board::serializeBitboard(wpSinglePushTargets(board, wPawns, empty), indBuf);
   for(uint16_t sq = 0; sq < numSquares; ++sq) {
      uint16_t toInd = indBuf[sq]; 
      moveList.emplace_back(QUIET_MOVE, toInd-singleRankOffset, toInd);
   }
   numSquares = Board::serializeBitboard(wpDoublePushTargets(board, wPawns, empty), indBuf);
   for(uint16_t sq = 0; sq < numSquares; ++sq) {
      uint16_t toInd = indBuf[sq]; 
      moveList.emplace_back(DOUBLE_PAWN_PUSH, toInd-doubleRankOffset, toInd);
   }
}

void appendBlackPawnPushMoves(Board* board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint64_t wPawns, uint64_t bPawns, uint64_t empty) {
   const int singleRankOffset = 8, doubleRankOffset = 16;

   uint16_t numSquares = Board::serializeBitboard(bpSinglePushTargets(board, bPawns, empty), indBuf);
   for(uint16_t sq = 0; sq < numSquares; ++sq) {
      uint16_t toInd = indBuf[sq]; 
      moveList.emplace_back(QUIET_MOVE, toInd+singleRankOffset, toInd);
   }
   numSquares = Board::serializeBitboard(bpDoublePushTargets(board, bPawns, empty), indBuf);
   for(uint16_t sq = 0; sq < numSquares; ++sq) {
      uint16_t toInd = indBuf[sq]; 
      moveList.emplace_back(DOUBLE_PAWN_PUSH, toInd+doubleRankOffset, toInd);
   }
}

void appendWhitePawnCaptureMoves(Board* board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint64_t wPawns, uint64_t bPawns, uint64_t empty) {
   uint16_t numSquares = Board::serializeBitboard(wPawns, indBuf);
   std::array<uint16_t, NUM_SQUARES> targetsBuf;
   for(uint16_t sq = 0; sq < numSquares; ++sq) {
      uint16_t fromInd = indBuf[sq];
      uint64_t targets = board->getWhitePawnAttacks(fromInd) & board->getPieceSet(Board::blackPieces);
      if(!targets) continue;
      appendFromBB(moveList, targetsBuf, fromInd, targets, CAPTURE);
   }
}

void appendBlackPawnCaptureMoves(Board* board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint64_t wPawns, uint64_t bPawns, uint64_t empty) {
   uint16_t numSquares = Board::serializeBitboard(bPawns, indBuf);
   std::array<uint16_t, NUM_SQUARES> targetsBuf;
   for(uint16_t sq = 0; sq < numSquares; ++sq) {
      uint16_t fromInd = indBuf[sq];
      uint64_t targets = board->getBlackPawnAttacks(fromInd) & board->getPieceSet(Board::whitePieces);
      if(!targets) continue;
      appendFromBB(moveList, targetsBuf, fromInd, targets, CAPTURE);
   }
}

void appendPawnMoves(Board* const board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, Turn turn) {
   uint64_t wPawns = board->getPieceSet(Board::whitePawns);
   uint64_t bPawns = board->getPieceSet(Board::blackPawns);
   uint64_t empty = board->getEmpty();

   if(turn == Turn::WHITE) {
      appendWhitePawnPushMoves(board, moveList, indBuf, wPawns, bPawns, empty);
      appendWhitePawnCaptureMoves(board, moveList, indBuf, wPawns, bPawns, empty);
   } else {
      appendBlackPawnPushMoves(board, moveList, indBuf, wPawns, bPawns, empty);
      appendBlackPawnCaptureMoves(board, moveList, indBuf, wPawns, bPawns, empty);
   }
}

void appendKnightMovesHelper(Board* const board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint64_t knights, uint64_t ownPieces, uint64_t oppPieces) {
   uint16_t numSquares = Board::serializeBitboard(knights, indBuf);
   std::array<uint16_t, NUM_SQUARES> targetsBuf;

   for(uint16_t sq = 0; sq < numSquares; ++sq) {
      uint16_t fromInd = indBuf[sq];
      uint64_t targets = board->getKnightMoves(fromInd) & ~ownPieces;
      if(!targets) continue;

      uint64_t attackTargets = targets & oppPieces;
      uint64_t quietTargets = targets ^ attackTargets;
      appendFromBB(moveList, targetsBuf, fromInd, quietTargets, QUIET_MOVE);
      appendFromBB(moveList, targetsBuf, fromInd, attackTargets, CAPTURE);
   }
}

void appendKnightMoves(Board* const board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, Turn turn) {
   uint64_t knights, ownPieces, oppPieces;
   if(turn == Turn::WHITE) {
      knights = board->getPieceSet(Board::whiteKnights);
      ownPieces = board->getPieceSet(Board::whitePieces);
      oppPieces = board->getPieceSet(Board::blackPieces);
   } else {
      knights = board->getPieceSet(Board::blackKnights);
      ownPieces = board->getPieceSet(Board::blackPieces);
      oppPieces = board->getPieceSet(Board::whitePieces);
   }

   appendKnightMovesHelper(board, moveList, indBuf, knights, ownPieces, oppPieces);
}

void appendKingMovesHelper(Board* const board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint16_t from, uint64_t ownPieces, uint64_t oppPieces) {
   uint64_t kingMoves = board->getKingMoves(from) & ~ownPieces;
   uint64_t attackTargets = kingMoves & oppPieces;
   uint64_t quietTargets = kingMoves ^ attackTargets;

   appendFromBB(moveList, indBuf, from, quietTargets, QUIET_MOVE);
   appendFromBB(moveList, indBuf, from, attackTargets, CAPTURE);
}

void appendKingMoves(Board* const board, std::vector<Move>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, Turn turn) {
   uint16_t from;
   uint64_t ownPieces, oppPieces;
   if(turn == Turn::WHITE) {
      from = Board::serializeSingleBit(board->getPieceSet(Board::whiteKing));
      ownPieces = board->getPieceSet(Board::whitePieces);
      oppPieces = board->getPieceSet(Board::blackPieces);
   } else {
      from = Board::serializeSingleBit(board->getPieceSet(Board::blackKing));
      ownPieces = board->getPieceSet(Board::blackPieces);
      oppPieces = board->getPieceSet(Board::whitePieces);
   }
   
   appendKingMovesHelper(board, moveList, indBuf, from, ownPieces, oppPieces);
}

std::vector<Move> MoveGen::getLegalMoves(Turn turn) {
   std::vector<Move> psuedoLegal;
   psuedoLegal.reserve(MAX_LEGAL_MOVES);
   std::array<uint16_t, NUM_SQUARES> indBuf;

   appendPawnMoves(m_board, psuedoLegal, indBuf, turn);
   appendKingMoves(m_board, psuedoLegal, indBuf, turn);
   appendKnightMoves(m_board, psuedoLegal, indBuf, turn);

   return psuedoLegal;
}