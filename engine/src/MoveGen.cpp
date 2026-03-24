#include "MoveGen.h"

using namespace Game;

void appendFromBB(MoveArr& moveArr, IndArr& moveList, uint16_t from, uint64_t targets, uint16_t flag) {
   Board::serializeBitboard(targets, moveList);
   for(int i = 0; i < moveList.size; ++i) {
      uint16_t to = moveList[i];
      moveArr.append(flag, from, to);
   }
}

uint64_t wpSinglePushTargets(Board* const board, uint64_t wPawns, uint64_t empty) {
   return board->shiftNorth(wPawns) & empty;
}

uint64_t wpDoublePushTargets(Board* const board, uint64_t wPawns, uint64_t empty) {
   uint64_t singlePushs = wpSinglePushTargets(board, wPawns, empty);
   return board->shiftNorth(wPawns) & empty & RANK_4;
}

uint64_t bpSinglePushTargets(Board* const board, uint64_t bPawns, uint64_t empty) {
   return board->shiftSouth(bPawns) & empty;
}

uint64_t bpDoublePushTargets(Board* const board, uint64_t bPawns, uint64_t empty) {
   uint64_t singlePushs = bpSinglePushTargets(board, bPawns, empty);
   return board->shiftSouth(bPawns) & empty & RANK_5;
}

void appendPawnPushMoves(Board* board, MoveArr& moveArr, uint64_t wPawns, uint64_t bPawns, uint64_t empty) {
   const int singleRankOffset = 8, doubleRankOffset = 16;

   IndArr wSinglePushTargetList;
   Board::serializeBitboard(wpSinglePushTargets(board, wPawns, empty), wSinglePushTargetList);
   IndArr wDoublePushTargetList;
   Board::serializeBitboard(wpDoublePushTargets(board, wPawns, empty), wDoublePushTargetList);
   for(int i = 0; i < wSinglePushTargetList.size; ++i) {
      uint16_t toInd = wSinglePushTargetList[i];
      moveArr.append(toInd, toInd-singleRankOffset, QUIET_MOVE);
   }
   for(int i = 0; i < wDoublePushTargetList.size; ++i) {
      uint16_t toInd = wDoublePushTargetList[i];
      moveArr.append(toInd, toInd-doubleRankOffset, DOUBLE_PAWN_PUSH);
   }

   IndArr bSinglePushTargetList;
   Board::serializeBitboard(wpSinglePushTargets(board, bPawns, empty), bSinglePushTargetList);
   IndArr bDoublePushTargetList;
   Board::serializeBitboard(wpDoublePushTargets(board, bPawns, empty), bDoublePushTargetList);
   for(int i = 0; i < bSinglePushTargetList.size; ++i) {
      uint16_t toInd = bSinglePushTargetList[i];
      moveArr.append(toInd, toInd+singleRankOffset, QUIET_MOVE);
   }
   for(int i = 0; i < bDoublePushTargetList.size; ++i) {
      uint16_t toInd = bDoublePushTargetList[i];
      moveArr.append(toInd, toInd+doubleRankOffset, DOUBLE_PAWN_PUSH);
   }
}

void appendPawnCaptureMoves(Board* board, MoveArr& moveArr, uint64_t wPawns, uint64_t bPawns, uint64_t empty) {
   IndArr wPawnInds, targetsList;
   Board::serializeBitboard(wPawns, wPawnInds);
   for(int i = 0; i < wPawnInds.size; ++i) {
      uint16_t from = wPawnInds[i];
      uint64_t targets = board->getWhitePawnAttacks(from) & board->getPieceSet(Board::blackPieces);
      if(!targets) continue;
      appendFromBB(moveArr, targetsList, from, targets, CAPTURE);
   }

   IndArr bPawnInds;
   targetsList.reset();
   Board::serializeBitboard(bPawns, bPawnInds);
   for(int i = 0; i < bPawnInds.size; ++i) {
      uint16_t from = bPawnInds[i];
      uint64_t targets = board->getBlackPawnAttacks(from) & board->getPieceSet(Board::whitePieces);
      if(!targets) continue;
      appendFromBB(moveArr, targetsList, from, targets, CAPTURE);
   }
}

void appendPawnMoves(Board* const board, MoveArr& moveArr) {
   uint64_t wPawns = board->getPieceSet(Board::whitePawns);
   uint64_t bPawns = board->getPieceSet(Board::blackPawns);
   uint64_t empty = board->getEmpty();

   appendPawnPushMoves(board, moveArr, wPawns, bPawns, empty);
   appendPawnCaptureMoves(board, moveArr, wPawns, bPawns, empty);
}

void appendKingMovesHelper(Board* const board, MoveArr& moveArr, uint16_t from, uint64_t ownPieces, uint64_t oppPieces) {
   uint64_t kingMoves = board->getKingMoves(from) & ~ownPieces;
   uint64_t attackTargets = kingMoves & oppPieces;
   uint64_t quietTargets = kingMoves ^ attackTargets;

   IndArr quietsList;
   IndArr attacksList;
   appendFromBB(moveArr, quietsList, from, quietTargets, QUIET_MOVE);
   appendFromBB(moveArr, attacksList, from, attackTargets, CAPTURE);
}

void appendKingMoves(Board* const board, MoveArr& moveArr) {
   uint16_t wFrom = Board::serializeSingleBit(board->getPieceSet(Board::whiteKing));
   uint16_t bFrom = Board::serializeSingleBit(board->getPieceSet(Board::blackKing));
   uint64_t wPieces = board->getPieceSet(Board::whitePieces);
   uint64_t bPieces = board->getPieceSet(Board::blackPieces);
   
   appendKingMovesHelper(board, moveArr, wFrom, wPieces, bPieces); //append white king moves
   appendKingMovesHelper(board, moveArr, bFrom, bPieces, wPieces); //append black king moves
}

void appendKnightMovesHelper(Board* const board, MoveArr& moveArr, uint64_t knights, uint64_t ownPieces, uint64_t oppPieces) {
   IndArr knightSquares;
   Board::serializeBitboard(knights, knightSquares);

   IndArr quietsList;
   IndArr attacksList;
   for(int i = 0; i < knightSquares.size; ++i) {
      uint16_t from = knightSquares[i];
      uint64_t targets = board->getKnightMoves(from) & ~ownPieces;
      if(!targets) continue;

      quietsList.reset(), attacksList.reset();
      uint64_t attackTargets = targets & oppPieces;
      uint64_t quietTargets = targets ^ attackTargets;
      appendFromBB(moveArr, quietsList, from, quietTargets, QUIET_MOVE);
      appendFromBB(moveArr, attacksList, from, attackTargets, CAPTURE);
   }
}

void appendKnightMoves(Board* const board, MoveArr& moveArr) {
   uint64_t wKnights = board->getPieceSet(Board::whiteKnights);
   uint64_t bKnights = board->getPieceSet(Board::blackKnights);
   uint64_t wPieces = board->getPieceSet(Board::whitePieces);
   uint64_t bPieces = board->getPieceSet(Board::blackPieces);
   
   appendKnightMovesHelper(board, moveArr, wKnights, wPieces, bPieces); //append white knight moves
   appendKnightMovesHelper(board, moveArr, bKnights, bPieces, wPieces); //append black knight moves
}

MoveArr MoveGen::getLegalMoves() {
   MoveArr psuedoLegal;
   appendPawnMoves(m_board, psuedoLegal);
   appendKingMoves(m_board, psuedoLegal);
   appendKnightMoves(m_board, psuedoLegal);

   return psuedoLegal;
}