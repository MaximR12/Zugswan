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

void appendKnightMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, uint64_t knights, uint64_t allInBetween, uint64_t checkMask) {
   knights &= ~allInBetween;
   moveTargets[Board::northNorthEast] = Board::shiftNorthNorthEast(knights) & checkMask; 
   moveTargets[Board::northEastEast] = Board::shiftNorthEastEast(knights) & checkMask;
   moveTargets[Board::northNorthWest] = Board::shiftNorthNorthWest(knights) & checkMask;
   moveTargets[Board::northWestWest] = Board::shiftNorthWestWest(knights) & checkMask;
   moveTargets[Board::southSouthEast] = Board::shiftSouthSouthEast(knights) & checkMask; 
   moveTargets[Board::southEastEast] = Board::shiftSouthEastEast(knights) & checkMask;
   moveTargets[Board::southSouthWest] = Board::shiftSouthSouthWest(knights) & checkMask;
   moveTargets[Board::southWestWest] = Board::shiftSouthWestWest(knights) & checkMask;
}

void appendPawnMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, std::array<uint64_t, NUM_ORTHOGONAL_DIRECTIONS>& promoMoveTargets, uint64_t pawns, 
      Board::Directions pawnDir, uint64_t oppPieces, uint64_t epAttackTargets, uint64_t verInBetween, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween,
      uint64_t empty, uint64_t checkMask, Board::PieceColor color) 
{
   //push moves
   uint64_t pinSafe = pawns & ~(allInBetween ^ verInBetween);
   uint64_t singlePushTargets = Board::pawnShift(pinSafe, pawnDir) & empty;
   moveTargets[pawnDir] |= singlePushTargets & checkMask & NOT_LAST_RANK;
   promoMoveTargets[pawnDir] = singlePushTargets & checkMask & LAST_RANK;
   promoMoveTargets[Board::getOppositeDirection(pawnDir)] = 0;

   uint64_t doublePushTargets = Board::pawnShift(pinSafe & PAWN_RANK, pawnDir) & empty;
   moveTargets[pawnDir] |= Board::pawnShift(doublePushTargets, pawnDir) & empty & checkMask;

   //attacks
   uint64_t northDirMask = Board::fullBoolMask(pawnDir == Board::north), southDirMask = ~northDirMask;
   uint64_t pawnAttackTargetsEast = Board::pawnAttackTargetsSafe(pawns, color, Board::east, diagInBetween, antiInBetween, allInBetween) & (oppPieces | epAttackTargets);
   uint64_t northEastTargets = pawnAttackTargetsEast & Board::shiftNorthEast(pawns) & checkMask & northDirMask;
   moveTargets[Board::northEast] |= northEastTargets & NOT_LAST_RANK;
   uint64_t southEastTargets = pawnAttackTargetsEast & Board::shiftSouthEast(pawns) & checkMask & southDirMask;
   moveTargets[Board::southEast] |= southEastTargets & NOT_LAST_RANK;
   promoMoveTargets[Board::east] = (northEastTargets | southEastTargets) & LAST_RANK; 

   uint64_t pawnAttackTargetsWest = Board::pawnAttackTargetsSafe(pawns, color, Board::west, diagInBetween, antiInBetween, allInBetween) & (oppPieces | epAttackTargets);
   uint64_t northWestTargets = pawnAttackTargetsWest & Board::shiftNorthWest(pawns) & checkMask & northDirMask;
   moveTargets[Board::northWest] |= northWestTargets & NOT_LAST_RANK;
   uint64_t southWestTargets = pawnAttackTargetsWest & Board::shiftSouthWest(pawns) & checkMask & southDirMask;
   moveTargets[Board::southWest] |= southWestTargets & NOT_LAST_RANK;
   promoMoveTargets[Board::west] = (northWestTargets | southWestTargets) & LAST_RANK;
}

void appendKingMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, uint64_t pieces, uint64_t king, uint64_t rooks, uint64_t oppAnyAttacks,
      uint64_t nullIfCheck, bool kingCastleRights, bool queenCastleRights, uint64_t occupied, uint64_t empty) 
{
   uint64_t targetMask = ~(pieces | oppAnyAttacks);
   moveTargets[Board::north] |= Board::shiftNorth(king) & targetMask;
   moveTargets[Board::south] |= Board::shiftSouth(king) & targetMask;
   moveTargets[Board::east] |= Board::shiftEast(king) & targetMask;
   moveTargets[Board::west] |= Board::shiftWest(king) & targetMask;
   moveTargets[Board::northEast] |= Board::shiftNorthEast(king) & targetMask;
   moveTargets[Board::northWest] |= Board::shiftNorthWest(king) & targetMask;
   moveTargets[Board::southEast] |= Board::shiftSouthEast(king) & targetMask;
   moveTargets[Board::southWest] |= Board::shiftSouthWest(king) & targetMask;

   //castle moves
   targetMask &= empty;
   uint64_t kingCastleMask = Board::fullBoolMask(kingCastleRights), queenCastleMask = Board::fullBoolMask(queenCastleRights);
   uint64_t eastOne = Board::shiftEast(king) & targetMask;
   moveTargets[Board::east] |= Board::shiftEast(eastOne) & targetMask & kingCastleMask & Board::fullBoolMask((king << 3) & rooks) & nullIfCheck;
   uint64_t westOne = Board::shiftWest(king) & targetMask;
   uint64_t westTwo = Board::shiftWest(westOne) & targetMask;
   moveTargets[Board::west] |= westTwo & Board::nullBoolMask(Board::shiftWest(westTwo) & occupied) & queenCastleMask & Board::fullBoolMask((king >> 4) & rooks) & nullIfCheck; //queen castle includes occupency check of square west of queen rook
}

uint16_t serializeKnightMoves(std::array<Move, MAX_LEGAL_MOVES>& moveBuf, std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS> targetInds, 
      std::array<uint16_t, NUM_TOTAL_DIRECTIONS> targetIndLengths, uint64_t oppPieces) 
{
   uint16_t currInd = 0;
   for(int dir = Board::northNorthEast; dir < NUM_TOTAL_DIRECTIONS; ++dir) { 
      uint16_t len = targetIndLengths[dir];
      for(int i = 0; i < len; ++i) {
         uint16_t to = targetInds[dir][i];
         uint16_t oppDirectionOffset = Board::getDirectionOffset(Board::getOppositeDirection(static_cast<Board::Directions>(dir)));
         uint16_t from = to + oppDirectionOffset; 

         int16_t nullIfCapture = ((int64_t)((1ULL << to) & oppPieces) - 1) >> 63;
         uint16_t flag = CAPTURE & ~nullIfCapture;

         moveBuf[currInd++] = Move(flag, from, to);
      }
   } 

   return currInd;
}

uint16_t getFlag(uint16_t from, uint16_t to, uint64_t pawns, uint64_t king, uint64_t oppPieces, uint64_t epTargets) {
   uint64_t toBB = 1ULL << to, fromBB = 1ULL << from;
   uint16_t flag = QUIET_MOVE;

   int64_t nullIfCapture = Board::nullBoolMask(toBB & oppPieces);
   flag |= CAPTURE & ~nullIfCapture;

   uint64_t fromIfPawn = fromBB & pawns;
   int64_t pawnMask = Board::fullBoolMask(fromIfPawn);

   uint64_t nonZeroIfDoublePush = ((toBB << 16) & fromBB) | ((toBB >> 16) & fromBB);
   flag |= DOUBLE_PAWN_PUSH & Board::fullBoolMask(nonZeroIfDoublePush) & pawnMask;
   flag |= EP_CAPTURE & Board::fullBoolMask(toBB & epTargets) & pawnMask;
   
   uint64_t nonZeroIfKingCastle = (toBB >> 2) & king & fromBB;
   uint64_t nonZeroIfQueenCastle = (toBB << 2) & king & fromBB;
   flag |= KING_CASTLE & Board::fullBoolMask(nonZeroIfKingCastle);
   flag |= QUEEN_CASTLE & Board::fullBoolMask(nonZeroIfQueenCastle);

   return flag;
}

uint16_t serializeSliderMoves(std::array<Move, MAX_LEGAL_MOVES>& moveBuf, uint16_t currInd, std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS>& targetInds, 
      std::array<uint16_t, NUM_TOTAL_DIRECTIONS>& targetIndLengths, std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS>& promoInds, 
      std::array<uint16_t, NUM_ORTHOGONAL_DIRECTIONS>& promoIndLengths, uint64_t pawns, Board::Directions pawnDir, uint64_t king, uint64_t oppPieces,
      uint64_t epTargets, uint64_t occupied) 
{
   for(int dir = 0; dir < NUM_SLIDER_DIRECTIONS; ++dir) { 
      uint16_t len = targetIndLengths[dir];
      for(int i = 0; i < len; ++i) {
         uint16_t to = targetInds[dir][i];
         uint16_t from = Board::serializeSingleBit(getRayAttacks(to, occupied, Board::getOppositeDirection(dir)) & occupied);
         uint16_t flag = getFlag(from, to, pawns, king, oppPieces, epTargets);

         moveBuf[currInd++] = Move(flag, from, to);
      }
   }

   for(int dir = 0; dir < NUM_ORTHOGONAL_DIRECTIONS; ++dir) { 
      uint16_t len = promoIndLengths[dir];
      for(int i = 0; i < len; ++i) {
         uint16_t to = promoInds[dir][i];
         uint16_t from = to - (Board::getDirectionOffset(pawnDir) + Board::getDirectionOffset(dir) % 8);

         int64_t nullIfCapture = Board::nullBoolMask((1ULL << to) & oppPieces);
         uint16_t captureMask = CAPTURE & ~nullIfCapture;
         for(uint16_t flag = KNIGHT_PROMOTION; flag <= QUEEN_PROMOTION; ++flag) //append each possible piece promo
            moveBuf[currInd++] = Move(flag | captureMask, from, to);
      }
   }

   return currInd;
}

uint16_t serializeMoves(std::array<Move, MAX_LEGAL_MOVES>& moveBuf, std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, std::array<uint64_t, NUM_ORTHOGONAL_DIRECTIONS>& promoMoveTargets,
      uint64_t pawns, Board::Directions pawnDir, uint64_t king, uint64_t oppPieces, uint64_t epTargets, uint64_t occupied) 
{
   std::array<uint16_t, NUM_TOTAL_DIRECTIONS> targetIndLengths; 
   std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS> targetInds;
   for(int dir = 0; dir < NUM_TOTAL_DIRECTIONS; ++dir) 
      targetIndLengths[dir] = Board::serializeBitboard(moveTargets[dir], targetInds[dir]);
   uint16_t currInd = serializeKnightMoves(moveBuf, targetInds, targetIndLengths, oppPieces);

   std::array<uint16_t, NUM_ORTHOGONAL_DIRECTIONS> promoIndLengths; 
   std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS> promoInds;
   for(int dir = 0; dir < NUM_ORTHOGONAL_DIRECTIONS; ++dir) 
      promoIndLengths[dir] = Board::serializeBitboard(promoMoveTargets[dir], promoInds[dir]);
   return serializeSliderMoves(moveBuf, currInd, targetInds, targetIndLengths, promoInds, promoIndLengths, pawns, pawnDir, king, oppPieces, epTargets, occupied);
}

//Dirgolem move generation, generate 16 move target bitboards for each direction, then serialize into move objects
uint16_t MoveGen::getLegalMoves(Board::PieceColor color, std::array<Move, MAX_LEGAL_MOVES>& moveBuf) const {
   std::array<uint64_t, NUM_TOTAL_DIRECTIONS> moveTargets;
   std::array<uint64_t, NUM_ORTHOGONAL_DIRECTIONS> promoMoveTargets; //pawn promotion moves

   Board::PieceColor oppColor = Board::getOppositeColor(color);
   Board::Directions pawnDir = Board::getPawnDirection(color);
   Board::Directions oppPawnDir = Board::getPawnDirection(oppColor);

   uint64_t pieces = m_board->getPieceSet(Board::all, color);
   uint64_t king = m_board->getPieceSet(Board::king, color);
   uint64_t pawns = m_board->getPieceSet(Board::pawns, color);
   uint64_t rooks = m_board->getPieceSet(Board::rooks, color);
   uint64_t rookLike = m_board->getPieceSet(Board::rooks, color) | m_board->getPieceSet(Board::queens, color);
   uint64_t bishopLike = m_board->getPieceSet(Board::bishops, color) | m_board->getPieceSet(Board::queens, color);
   uint64_t knights = m_board->getPieceSet(Board::knights, color);
   uint64_t epTarget = m_board->getEnPassantTargets(color);
   uint64_t oppPieces = m_board->getPieceSet(Board::all, oppColor);
   uint64_t oppKing = m_board->getPieceSet(Board::king, oppColor);
   uint64_t oppPawns = m_board->getPieceSet(Board::pawns, oppColor);
   uint64_t oppKnights = m_board->getPieceSet(Board::knights, oppColor);
   uint64_t oppRookLike = m_board->getPieceSet(Board::rooks, oppColor) | m_board->getPieceSet(Board::queens, oppColor);
   uint64_t oppBishopLike = m_board->getPieceSet(Board::bishops, oppColor) | m_board->getPieceSet(Board::queens, oppColor);

   uint16_t kingInd = Board::serializeSingleBit(king);
   uint64_t empty = m_board->getEmpty();
   uint64_t occupied = m_board->getOccupied();

   uint64_t oppSliderNorth = Board::northFill(oppRookLike, empty | king);
   uint64_t oppSliderSouth = Board::southFill(oppRookLike, empty | king);
   uint64_t oppSliderEast = Board::eastFill(oppRookLike, empty | king);
   uint64_t oppSliderWest = Board::westFill(oppRookLike, empty | king);
   uint64_t oppSliderNorthEast = Board::northEastFill(oppBishopLike, empty | king);
   uint64_t oppSliderNorthWest = Board::northWestFill(oppBishopLike, empty | king);
   uint64_t oppSliderSouthEast = Board::southEastFill(oppBishopLike, empty | king);
   uint64_t oppSliderSouthWest = Board::southWestFill(oppBishopLike, empty | king);

   uint64_t horSliderAttacks = oppSliderEast | oppSliderWest;
   uint64_t verSliderAttacks = oppSliderNorth | oppSliderSouth;
   uint64_t diagSliderAttacks = oppSliderNorthEast | oppSliderSouthWest;
   uint64_t antiSliderAttacks = oppSliderNorthWest | oppSliderSouthEast;

   uint64_t kingNorth = Board::northFill(king, empty);
   uint64_t kingSouth = Board::southFill(king, empty);
   uint64_t kingEast = Board::eastFill(king, empty);
   uint64_t kingWest = Board::westFill(king, empty);
   uint64_t kingNorthEast = Board::northEastFill(king, empty);
   uint64_t kingSouthWest = Board::southWestFill(king, empty);
   uint64_t kingNorthWest = Board::northWestFill(king, empty);
   uint64_t kingSouthEast = Board::southEastFill(king, empty);
   
   uint64_t verInBetween = oppSliderNorth & kingSouth;
   verInBetween |= oppSliderSouth & kingNorth;
   uint64_t horInBetween = oppSliderEast & kingWest;
   horInBetween |= oppSliderWest & kingEast;
   uint64_t diagInBetween = oppSliderNorthEast & kingSouthWest;
   diagInBetween |= oppSliderSouthWest & kingNorthEast;
   uint64_t antiInBetween = oppSliderNorthWest & kingSouthEast;
   antiInBetween |= oppSliderSouthEast & kingNorthWest;

   uint64_t kingSuperOrth = kingNorth | kingSouth | kingEast | kingWest;
   uint64_t kingSuperDiag = kingNorthEast | kingNorthWest | kingSouthEast | kingSouthWest;
   uint64_t allInBetween = horInBetween | verInBetween | diagInBetween | antiInBetween;

   uint64_t oppPawnAttacks = Board::pawnAttackTargets(oppPawns, oppColor);
   uint64_t pawnCheckFrom = m_tables->getPawnSquareAttacks(kingInd, color) & oppPawns;

   uint64_t oppAnyAttacks = oppPawnAttacks | horSliderAttacks | verSliderAttacks | diagSliderAttacks | antiSliderAttacks 
         | Board::knightAttackTargets(oppKnights) | Board::kingAttackTargets(oppKing);

   uint64_t blocks = allInBetween & empty;
   uint64_t checkFrom = (kingSuperOrth & oppRookLike) | (kingSuperDiag & oppBishopLike) 
         | pawnCheckFrom | (m_tables->getKnightMoves(kingInd) & oppKnights);

   int64_t nullIfCheck = Board::nullBoolMask(oppAnyAttacks & king);
   int64_t nullIfDoubleCheck = Board::nullBoolMask(checkFrom & (checkFrom - 1));
   uint64_t checkTo = checkFrom | blocks | nullIfCheck;
   uint64_t checkMask = ~pieces & checkTo & nullIfDoubleCheck;
   uint64_t pawnCheckMask = checkMask | (epTarget & Board::pawnShift(checkFrom, pawnDir));

   appendHorSliderMoves(moveTargets, horInBetween, allInBetween, rookLike, empty, checkMask); //queen + rook
   appendVerSliderMoves(moveTargets, verInBetween, allInBetween, rookLike, empty, checkMask); //queen + rook
   appendDiagSliderMoves(moveTargets, diagInBetween, allInBetween, bishopLike, empty, checkMask); //queen + bishop
   appendAntiSliderMoves(moveTargets, antiInBetween, allInBetween, bishopLike, empty, checkMask); //queen + bishop
   appendKnightMoves(moveTargets, knights, allInBetween, checkMask);
   appendPawnMoves(moveTargets, promoMoveTargets, pawns, pawnDir, oppPieces, epTarget, verInBetween, diagInBetween, antiInBetween, allInBetween, empty, pawnCheckMask, color);
   appendKingMoves(moveTargets, pieces, king, rooks, oppAnyAttacks, nullIfCheck, m_board->getKingCastleRights(color), m_board->getQueenCastleRights(color), occupied, empty);

   return serializeMoves(moveBuf, moveTargets, promoMoveTargets, pawns, pawnDir, king, oppPieces, epTarget, m_board->getOccupied());
}