#include "MoveGen.h"
#include <array>
#include <unordered_map>

uint64_t getPositiveRayAttacks(uint16_t square, uint64_t occupied, Board::Directions dir) {
   uint64_t attacks = Board::getRayMoves(square, dir);
   uint64_t blocker = attacks & occupied;
   if(blocker) {
      square = Board::bitScanForward(blocker);
      attacks ^= Board::getRayMoves(square, dir);
   }

   return attacks; 
}

uint64_t getNegativeRayAttacks(uint16_t square, uint64_t occupied, Board::Directions dir) {
   uint64_t attacks = Board::getRayMoves(square, dir);
   uint64_t blocker = attacks & occupied;
   if(blocker) {
      square = Board::bitScanReverse(blocker);
      attacks ^= Board::getRayMoves(square, dir);
   }

   return attacks; 
}

uint64_t getRayAttacks(uint16_t square, uint64_t occupied, Board::Directions dir) {
   uint64_t attacks = Board::getRayMoves(square, dir);
   uint64_t blocker = attacks & occupied;
   if(blocker) {
      square = Board::bitScan(blocker, Board::isNegative(dir));
      attacks ^= Board::getRayMoves(square, dir);
   }

   return attacks; 
}

uint64_t getDiagAttacks(uint16_t square, uint64_t occupied) {
   return getPositiveRayAttacks(square, occupied, Board::Directions::northEast)
         | getNegativeRayAttacks(square, occupied, Board::Directions::southWest);
}

uint64_t getAntiDiagAttacks(uint16_t square, uint64_t occupied) {
   return getPositiveRayAttacks(square, occupied, Board::Directions::northWest)
         | getNegativeRayAttacks(square, occupied, Board::Directions::southEast);
}

uint64_t getFileAttacks(uint16_t square, uint64_t occupied) {
   return getPositiveRayAttacks(square, occupied, Board::Directions::north)
         | getNegativeRayAttacks(square, occupied, Board::Directions::south);
}

uint64_t getRankAttacks(uint16_t square, uint64_t occupied) {
   return getPositiveRayAttacks(square, occupied, Board::Directions::east)
         | getNegativeRayAttacks(square, occupied, Board::Directions::west);
}

uint64_t getHorSliderAttacks(uint64_t rookLike, uint64_t empty) {
   return Board::westFill(rookLike, empty) | Board::eastFill(rookLike, empty);
}

uint64_t getVerSliderAttacks(uint64_t rookLike, uint64_t empty) {
   return Board::northFill(rookLike, empty) | Board::southFill(rookLike, empty);
}

uint64_t getDiagSliderAttacks(uint64_t bishopLike, uint64_t empty) {
   return Board::northEastFill(bishopLike, empty) | Board::southWestFill(bishopLike, empty);
}

uint64_t getAntiSliderAttacks(uint64_t bishopLike, uint64_t empty) {
   return Board::northWestFill(bishopLike, empty) | Board::southEastFill(bishopLike, empty);
}

uint64_t getKingSuperOrth(uint64_t king, uint64_t empty) {
   return Board::northFill(king, empty) | Board::southFill(king, empty) | Board::westFill(king, empty) | Board::eastFill(king, empty);
}

uint64_t getKingSuperDiag(uint64_t king, uint64_t empty) {
   return Board::northEastFill(king, empty) | Board::northWestFill(king, empty) | Board::southEastFill(king, empty) | Board::southWestFill(king, empty);
}

void appendHorSliderMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, 
      uint64_t horInBetween, uint64_t allInBetween, uint64_t rookLike, uint64_t empty, uint64_t checkMask) 
{
   rookLike &= ~(allInBetween ^ horInBetween); //exclude pieces pinned in non-horizontal direction
   moveTargets[Board::east] = Board::eastFill(rookLike, empty) & checkMask;
   moveTargets[Board::west] = Board::westFill(rookLike, empty) & checkMask;
}

void appendVerSliderMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, 
      uint64_t verInBetween, uint64_t allInBetween, uint64_t rookLike, uint64_t empty, uint64_t checkMask) 
{
   rookLike &= ~(allInBetween ^ verInBetween); //exclude pieces pinned in non-vertical direction
   moveTargets[Board::north] = Board::northFill(rookLike, empty) & checkMask;
   moveTargets[Board::south] = Board::southFill(rookLike, empty) & checkMask;
}

void appendDiagSliderMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, 
      uint64_t diagInBetween, uint64_t allInBetween, uint64_t bishopLike, uint64_t empty, uint64_t checkMask) 
{
   bishopLike &= ~(allInBetween ^ diagInBetween); //exclude pieces pinned in non-diagonal direction
   moveTargets[Board::northEast] = Board::northEastFill(bishopLike, empty) & checkMask;
   moveTargets[Board::southWest] = Board::southWestFill(bishopLike, empty) & checkMask;
}

void appendAntiSliderMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, 
      uint64_t antiInBetween, uint64_t allInBetween, uint64_t bishopLike, uint64_t empty, uint64_t checkMask) 
{
   bishopLike &= ~(allInBetween ^ antiInBetween); //exclude pieces pinned in non-antidiagonal direction
   moveTargets[Board::northWest] = Board::northWestFill(bishopLike, empty) & checkMask;
   moveTargets[Board::southEast] = Board::southEastFill(bishopLike, empty) & checkMask;
}

void appendKnightMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, uint64_t knights, uint64_t checkMask) 
{
   moveTargets[Board::northNorthEast] = Board::shiftNorthNorthEast(knights) & checkMask; 
   moveTargets[Board::northEastEast] = Board::shiftNorthEastEast(knights) & checkMask;
   moveTargets[Board::northNorthWest] = Board::shiftNorthNorthWest(knights) & checkMask;
   moveTargets[Board::northWestWest] = Board::shiftNorthWestWest(knights) & checkMask;
   moveTargets[Board::southSouthEast] = Board::shiftSouthSouthEast(knights) & checkMask; 
   moveTargets[Board::southEastEast] = Board::shiftSouthEastEast(knights) & checkMask;
   moveTargets[Board::southSouthWest] = Board::shiftSouthSouthWest(knights) & checkMask;
   moveTargets[Board::southWestWest] = Board::shiftSouthWestWest(knights) & checkMask;
}

void serializeKnightMoves(std::vector<Move>& moves, std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS> targetInds, 
      std::array<uint16_t, NUM_TOTAL_DIRECTIONS> targetIndLengths, uint64_t oppPieces) 
{
   for(int dir = Board::northNorthEast; dir < NUM_TOTAL_DIRECTIONS; ++dir) { 
      uint16_t len = targetIndLengths[dir];
      for(int i = 0; i < len; ++i) {
         uint16_t to = targetInds[dir][i];
         uint16_t oppDirectionOffset = Board::getDirectionOffset(Board::getOppositeDirection(static_cast<Board::Directions>(dir)));
         uint16_t from = to + oppDirectionOffset; 

         int16_t nullIfCapture = ((int64_t)((1ULL << to) & oppPieces) - 1) >> 63;
         uint16_t flag = CAPTURE & ~nullIfCapture;

         moves.emplace_back(flag, from, to);
      }
   } 
}

void serializeSliderMoves(std::vector<Move>& moves, std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS> targetInds, 
      std::array<uint16_t, NUM_TOTAL_DIRECTIONS> targetIndLengths, uint64_t oppPieces, uint64_t occupied) 
{
   for(int dir = 0; dir < NUM_SLIDER_DIRECTIONS; ++dir) { 
      uint16_t len = targetIndLengths[dir];
      for(int i = 0; i < len; ++i) {
         uint16_t to = targetInds[dir][i];
         uint16_t from = Board::serializeSingleBit(getRayAttacks(to, occupied, Board::getOppositeDirection(dir)) & occupied);
         
         int16_t nullIfCapture = ((int64_t)((1ULL << to) & oppPieces) - 1) >> 63;
         uint16_t flag = CAPTURE & ~nullIfCapture;

         moves.emplace_back(flag, from, to);
      }
   } 
}

void serializeMoves(std::vector<Move>& moves, std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, uint64_t oppPieces, uint64_t occupied) {
   std::array<uint16_t, NUM_TOTAL_DIRECTIONS> targetIndLengths; 
   std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS> targetInds;
   for(int dir = 0; dir < NUM_TOTAL_DIRECTIONS; ++dir) 
      targetIndLengths[dir] = Board::serializeBitboard(moveTargets[dir], targetInds[dir]);

   serializeKnightMoves(moves, targetInds, targetIndLengths, oppPieces);
   serializeSliderMoves(moves, targetInds, targetIndLengths, oppPieces, occupied);
}

//Dirgolem move generation, generate 16 move target bitboards for each direction, then serialize into moves
std::vector<Move> MoveGen::getLegalMoves(Turn turn) {
   std::array<uint64_t, NUM_TOTAL_DIRECTIONS> moveTargets;

   uint64_t pieces, king, rookLike, bishopLike, knights, oppPieces, oppKing, oppKnights, oppRookLike, oppBishopLike;
   if(turn == Turn::WHITE) {
      pieces = m_board->getPieceSet(Board::PieceType::whitePieces);
      king = m_board->getPieceSet(Board::PieceType::whiteKing);
      rookLike = m_board->getPieceSet(Board::PieceType::whiteRooks) | m_board->getPieceSet(Board::PieceType::whiteQueens);
      bishopLike = m_board->getPieceSet(Board::PieceType::whiteBishops) | m_board->getPieceSet(Board::PieceType::whiteQueens);
      knights = m_board->getPieceSet(Board::PieceType::whiteKnights);
      oppPieces = m_board->getPieceSet(Board::PieceType::blackPieces);
      oppKing = m_board->getPieceSet(Board::PieceType::blackKing);
      oppKnights = m_board->getPieceSet(Board::PieceType::blackKnights);
      oppRookLike = m_board->getPieceSet(Board::PieceType::blackRooks) | m_board->getPieceSet(Board::PieceType::blackQueens);
      oppBishopLike = m_board->getPieceSet(Board::PieceType::blackBishops) | m_board->getPieceSet(Board::PieceType::blackQueens);
   } else {
      pieces = m_board->getPieceSet(Board::PieceType::blackPieces);
      king = m_board->getPieceSet(Board::PieceType::blackKing);
      rookLike = m_board->getPieceSet(Board::PieceType::blackRooks) | m_board->getPieceSet(Board::PieceType::blackQueens);
      bishopLike = m_board->getPieceSet(Board::PieceType::blackBishops) | m_board->getPieceSet(Board::PieceType::blackQueens);
      knights = m_board->getPieceSet(Board::PieceType::blackKnights);
      oppPieces = m_board->getPieceSet(Board::PieceType::whitePieces);
      oppKing = m_board->getPieceSet(Board::PieceType::whiteKing);
      oppKnights = m_board->getPieceSet(Board::PieceType::whiteKnights);
      oppRookLike = m_board->getPieceSet(Board::PieceType::whiteRooks) | m_board->getPieceSet(Board::PieceType::whiteQueens);
      oppBishopLike = m_board->getPieceSet(Board::PieceType::whiteBishops) | m_board->getPieceSet(Board::PieceType::whiteQueens);
   }

   uint16_t kingInd = Board::serializeSingleBit(king);
   uint64_t empty = m_board->getEmpty();
   uint64_t kingSuperOrth = getKingSuperOrth(king, empty);
   uint64_t kingSuperDiag = getKingSuperDiag(king, empty);

   uint64_t horSliderAttacks = getHorSliderAttacks(oppRookLike, empty);
   uint64_t verSliderAttacks = getVerSliderAttacks(oppRookLike, empty);
   uint64_t diagSliderAttacks = getDiagSliderAttacks(oppBishopLike, empty);
   uint64_t antiSliderAttacks = getAntiSliderAttacks(oppBishopLike, empty);

   uint64_t oppPawns, oppPawnAttacks, pawnCheckFrom;
   if(turn == Turn::WHITE) {
      oppPawns = m_board->getPieceSet(Board::blackPawns);
      oppPawnAttacks = Board::bpAttackTargets(oppPawns);
      pawnCheckFrom = m_board->getWhitePawnAttacks(kingInd) & oppPawns;
   } else {
      oppPawns = m_board->getPieceSet(Board::whitePawns);
      oppPawnAttacks = Board::wpAttackTargets(oppPawns);
      pawnCheckFrom = m_board->getBlackPawnAttacks(kingInd) & oppPawns;
   }

   uint64_t oppAnyAttacks = oppPawnAttacks | horSliderAttacks | verSliderAttacks | diagSliderAttacks | antiSliderAttacks 
         | Board::knightAttackTargets(oppKnights) | Board::kingAttackTargets(oppKing);

   uint64_t horInBetween = horSliderAttacks & kingSuperOrth;
   uint64_t verInBetween = verSliderAttacks & kingSuperOrth;
   uint64_t diagInBetween = diagSliderAttacks & kingSuperDiag;
   uint64_t antiInBetween = antiSliderAttacks & kingSuperDiag;
   uint64_t allInBetween = horInBetween | verInBetween | diagInBetween | antiInBetween;

   uint64_t blocks = allInBetween & empty;
   uint64_t checkFrom = (kingSuperOrth & oppRookLike) | (kingSuperDiag & oppBishopLike) 
         | pawnCheckFrom | (m_board->getKnightMoves(kingInd) & oppKnights);

   int64_t nullIfCheck = ((int64_t)(oppAnyAttacks & king) - 1) >> 63;
   int64_t nullIfDoubleCheck = ((int64_t)(checkFrom & (checkFrom - 1)) - 1) >> 63;
   uint64_t checkTo = checkFrom | blocks | nullIfCheck;
   uint64_t checkMask = ~pieces & checkTo & nullIfDoubleCheck;

   appendHorSliderMoves(moveTargets, horInBetween, allInBetween, rookLike, empty, checkMask);
   appendVerSliderMoves(moveTargets, verInBetween, allInBetween, rookLike, empty, checkMask);
   appendDiagSliderMoves(moveTargets, diagInBetween, allInBetween, bishopLike, empty, checkMask);
   appendAntiSliderMoves(moveTargets, antiInBetween, allInBetween, bishopLike, empty, checkMask);
   appendKnightMoves(moveTargets, knights, checkMask);
   
   std::vector<Move> moves;
   moves.reserve(MAX_LEGAL_MOVES);
   serializeMoves(moves, moveTargets, oppPieces, m_board->getOccupied());

   return moves;
}