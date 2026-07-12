#include <array>
#include <unordered_map>
#include "movegen.hpp"

uint64_t getPositiveRayAttacks(uint16_t square, uint64_t occupied, Board::Directions dir) {
   uint64_t attacks = Tables::getRayMoves(square, dir);
   uint64_t blocker = attacks & occupied;
   if(blocker) {
      square = Board::bitScanForward(blocker);
      attacks ^= Tables::getRayMoves(square, dir);
   }

   return attacks; 
}

uint64_t getNegativeRayAttacks(uint16_t square, uint64_t occupied, Board::Directions dir) {
   uint64_t attacks = Tables::getRayMoves(square, dir);
   uint64_t blocker = attacks & occupied;
   if(blocker) {
      square = Board::bitScanReverse(blocker);
      attacks ^= Tables::getRayMoves(square, dir);
   }

   return attacks; 
}

uint64_t getRayAttacks(uint16_t square, uint64_t occupied, Board::Directions dir) {
   uint64_t attacks = Tables::getRayMoves(square, dir);
   uint64_t blocker = attacks & occupied;
   if(blocker) {
      square = Board::bitScan(blocker, Board::isNegative(dir));
      attacks ^= Tables::getRayMoves(square, dir);
   }

   return attacks; 
}

template<Board::PieceColor color, MoveType type>
uint16_t getFlag(Board* board, uint16_t from, uint16_t to) {
   constexpr Board::PieceColor oppColor = color == Board::white ? Board::black : Board::white;

   uint64_t oppPieces = board->getPieceSet(Board::all, oppColor);
   uint64_t toBB = 1ULL << to, fromBB = 1ULL << from;
   
   uint16_t flag = QUIET_MOVE;
   int64_t captureMask = Board::fullBoolMask(toBB & oppPieces);
   flag |= CAPTURE & captureMask;

   if constexpr (type == MoveType::pawn) {
      uint64_t pawns = board->getPieceSet(Board::pawns, color);
      uint64_t epTargets = board->getEnPassantTarget(color);
      uint64_t fromIfPawn = fromBB & pawns;
      int64_t pawnMask = Board::fullBoolMask(fromIfPawn);

      uint64_t nonZeroIfDoublePush = ((toBB << 16) & fromBB) | ((toBB >> 16) & fromBB);
      flag |= DOUBLE_PAWN_PUSH & Board::fullBoolMask(nonZeroIfDoublePush) & pawnMask;
      flag |= EP_CAPTURE & Board::fullBoolMask(toBB & epTargets) & pawnMask;
   }
   
   if constexpr (type == MoveType::king) {
      uint64_t king = board->getPieceSet(Board::king, color);

      uint64_t nonZeroIfKingCastle = (toBB >> 2) & king & fromBB;
      uint64_t nonZeroIfQueenCastle = (toBB << 2) & king & fromBB;
      flag |= KING_CASTLE & Board::fullBoolMask(nonZeroIfKingCastle);
      flag |= QUEEN_CASTLE & Board::fullBoolMask(nonZeroIfQueenCastle);
   }

   return flag;
}

template<Board::PieceColor color, MoveType type>
void serializeMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, uint64_t targets, int16_t from) 
{
   int16_t fromOffset;
   if constexpr (type == MoveType::pawn || type == MoveType::promotion)
      fromOffset = from; //from acts as offset for pawn moves

   uint64_t occupied = board->getOccupied();
   std::array<uint16_t, NUM_SQUARES> indices;
   int len = Board::serializeBitboard(targets, indices);

   for(int i = 0; i < len; ++i) {
      uint16_t to = indices[i];
      if constexpr (type == MoveType::pawn || type == MoveType::promotion) 
         from = to + fromOffset; 
      uint16_t flag = getFlag<color, type>(board, from, to);

      if constexpr (type != MoveType::promotion)
         moveList.push_back(Move(flag, from, to));
      else {
         moveList.push_back(Move(KNIGHT_PROMOTION | flag, from, to));
         moveList.push_back(Move(BISHOP_PROMOTION | flag, from, to));
         moveList.push_back(Move(ROOK_PROMOTION | flag, from, to));
         moveList.push_back(Move(QUEEN_PROMOTION | flag, from, to));
      }
   }
}

template<Board::PieceColor color>
void appendSliderMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf,
      std::array<uint64_t, NUM_MASK_TYPES>& masks, std::array<uint64_t, NUM_SQUARES>& pinMasks)
{
   uint64_t occupied = board->getOccupied();
   uint64_t rookLike = board->getPieceSet(Board::rooks, color) | board->getPieceSet(Board::queens, color);
   uint64_t bishopLike = board->getPieceSet(Board::bishops, color) | board->getPieceSet(Board::queens, color);

   uint16_t numRookLike = Board::serializeBitboard(rookLike, indBuf);
   for(int i = 0; i < numRookLike; ++i) {
      uint16_t square = indBuf[i];
      uint64_t squareBB = 1ULL<<square;
      uint64_t psuedoLegal = Tables::rookAttacks(square, occupied);

      uint64_t pinMask = UNIVERSE;
      if(squareBB & masks[Board::pinned])
         pinMask = pinMasks[square];

      uint64_t legal = psuedoLegal & pinMask & masks[Board::targetMask];
      if(legal) serializeMoves<color, MoveType::slider>(board, moveList, legal, square);
   }

   uint16_t numBishopLike = Board::serializeBitboard(bishopLike, indBuf);
   for(int i = 0; i < numBishopLike; ++i) {
      uint16_t square = indBuf[i];
      uint64_t squareBB = 1ULL<<square;
      uint64_t psuedoLegal = Tables::bishopAttacks(square, occupied);

      uint64_t pinMask = UNIVERSE;
      if(squareBB & masks[Board::pinned])
         pinMask = pinMasks[square];
      
      uint64_t legal = psuedoLegal & pinMask & masks[Board::targetMask];
      if(legal) serializeMoves<color, MoveType::slider>(board, moveList, legal, square);
   }
}

template<Board::PieceColor color>
void appendKnightMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf,
      std::array<uint64_t, NUM_MASK_TYPES>& masks)
{
   uint64_t knights = board->getPieceSet(Board::knights, color);
   knights &= ~masks[Board::pinned];

   uint64_t numKnights = Board::serializeBitboard(knights, indBuf);
   for(int i = 0; i < numKnights; ++i) {
      uint16_t square = indBuf[i];
      uint64_t psuedoLegal = Tables::knightMoveTable[square];
      
      uint64_t legal = psuedoLegal & masks[Board::targetMask];
      if(legal) serializeMoves<color, MoveType::knight>(board, moveList, legal, square);
   }
}

template<Board::PieceColor color>
void appendPawnMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf,
      std::array<uint64_t, NUM_MASK_TYPES>& masks) 
{
   constexpr Board::PieceColor oppColor = color == Board::white ? Board::black : Board::white;
   constexpr uint64_t pawnRank = color == Board::white ? RANK_2 : RANK_7;

   constexpr Board::MaskTypes leftDiag = color == Board::white ? Board::antiMovable : Board::diagMovable;
   constexpr Board::MaskTypes rightDiag = color == Board::white ? Board::diagMovable : Board::antiMovable;

   constexpr Board::Directions up = color == Board::white ? Board::north : Board::south;
   constexpr Board::Directions upRight = color == Board::white ? Board::northEast : Board::southEast;
   constexpr Board::Directions upLeft = color == Board::white ? Board::northWest : Board::southWest;

   constexpr int16_t downOffset = color == Board::white ? southOffset : northOffset;
   constexpr int16_t downLeftOffset = color == Board::white ? southWestOffset : northWestOffset;
   constexpr int16_t downRightOffset = color == Board::white ? southEastOffset : northEastOffset;

   uint64_t pawns = board->getPieceSet(Board::pawns, color);
   uint64_t oppPieces = board->getPieceSet(Board::all, oppColor);
   uint64_t epAttackTarget = board->getEnPassantTarget(color);
   uint64_t empty = board->getEmpty();
   uint64_t targetMask = masks[Board::pawnTargetMask]; 

   uint64_t pushable = pawns & masks[Board::verMovable];
   uint64_t singlePushTargets = Board::shift<up>(pushable) & empty & targetMask;
   uint64_t doublePushTargets = Board::shift<up>(pushable & pawnRank) & empty;
   doublePushTargets = Board::shift<up>(doublePushTargets) & empty & targetMask;
   uint64_t leftAttacks = Board::shift<upLeft>(pawns & masks[leftDiag]) & (oppPieces | epAttackTarget) & targetMask;
   uint64_t rightAttacks = Board::shift<upRight>(pawns & masks[rightDiag]) & (oppPieces | epAttackTarget) & targetMask;
   
   if(singlePushTargets & NOT_LAST_RANK) serializeMoves<color, MoveType::pawn>(board, moveList, singlePushTargets & NOT_LAST_RANK, downOffset);
   if(doublePushTargets) serializeMoves<color, MoveType::pawn>(board, moveList, doublePushTargets, downOffset*2);
   if(rightAttacks & NOT_LAST_RANK) serializeMoves<color, MoveType::pawn>(board, moveList, rightAttacks & NOT_LAST_RANK, downLeftOffset);
   if(leftAttacks & NOT_LAST_RANK) serializeMoves<color, MoveType::pawn>(board, moveList, leftAttacks & NOT_LAST_RANK, downRightOffset);

   if(singlePushTargets & LAST_RANK) serializeMoves<color, MoveType::promotion>(board, moveList, singlePushTargets & LAST_RANK, downOffset);
   if(rightAttacks & LAST_RANK) serializeMoves<color, MoveType::promotion>(board, moveList, rightAttacks & LAST_RANK, downLeftOffset);
   if(leftAttacks & LAST_RANK) serializeMoves<color, MoveType::promotion>(board, moveList, leftAttacks & LAST_RANK, downRightOffset);
}

template<Board::PieceColor color>
void appendKingMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, std::array<uint64_t, NUM_MASK_TYPES>& masks) {
   uint64_t targetMask = masks[Board::kingTargetMask];
   uint64_t king = board->getPieceSet(Board::king, color);
   uint16_t kingSquare = Board::serializeSingleBit(king);
   
   uint64_t targets = Tables::kingMoveTable[kingSquare] & targetMask;
   if(targets) serializeMoves<color, MoveType::king>(board, moveList, targets, kingSquare);

   bool kingCastleRights = board->getKingCastleRights(color);
   bool queenCastleRights = board->getQueenCastleRights(color);

   if(!kingCastleRights && !queenCastleRights)
      return;

   uint64_t rooks = board->getPieceSet(Board::rooks, color);
   uint64_t occupied = board->getOccupied();
   uint64_t empty = board->getEmpty();
   uint64_t notInCheck = masks[Board::notInCheck];

   targetMask &= empty;
   uint64_t kingCastleMask = Board::fullBoolMask(kingCastleRights), queenCastleMask = Board::fullBoolMask(queenCastleRights);
   uint64_t eastOne = Board::shift<Board::east>(king) & targetMask;
   uint64_t leftCastle = Board::shift<Board::east>(eastOne) & targetMask & kingCastleMask & Board::fullBoolMask((king << 3) & rooks) & notInCheck;
   uint64_t westOne = Board::shift<Board::west>(king) & targetMask;
   uint64_t westTwo = Board::shift<Board::west>(westOne) & targetMask;
   uint64_t rightCastle = westTwo & Board::nullBoolMask(Board::shift<Board::west>(westTwo) & occupied) & queenCastleMask & Board::fullBoolMask((king >> 4) & rooks) & notInCheck; //queen castle includes occupency check of square west of queen rook

   uint64_t castleTargets = leftCastle | rightCastle;
   if(castleTargets) serializeMoves<color, MoveType::king>(board, moveList, castleTargets, kingSquare);
}

void populatePinMasks(std::array<uint64_t, NUM_SQUARES>& pinMasks, std::array<uint16_t, NUM_SQUARES>& indBuf, Board::SliderRays dir, uint64_t inBetween) {
   uint16_t num = Board::serializeBitboard(inBetween, indBuf);
   for(int i = 0; i < num; ++i) {
      uint16_t ind = indBuf[i];
      pinMasks[ind] = Tables::getSliderMoves(ind, dir);
   }
}

//populates mask array
template<Board::PieceColor color>
void generateMasks(Board* board, std::array<uint64_t, NUM_MASK_TYPES>& masks, std::array<uint64_t, NUM_SQUARES>& pinMasks, std::array<uint16_t, NUM_SQUARES>& indBuf) {
   constexpr Board::PieceColor oppColor = color == Board::white ? Board::black : Board::white;
   constexpr Board::Directions up = color == Board::white ? Board::north : Board::south;

   uint64_t empty = board->getEmpty();
   uint64_t occupied = board->getOccupied();
   uint64_t pieces = board->getPieceSet(Board::all, color);
   uint64_t king = board->getPieceSet(Board::king, color);
   uint16_t kingSquare = Board::serializeSingleBit(king);
   uint64_t oppKing = board->getPieceSet(Board::king, oppColor);
   uint64_t oppPawns = board->getPieceSet(Board::pawns, oppColor);
   uint64_t oppKnights = board->getPieceSet(Board::knights, oppColor);
   uint64_t oppRookLike = board->getPieceSet(Board::rooks, oppColor) | board->getPieceSet(Board::queens, oppColor);
   uint64_t oppBishopLike = board->getPieceSet(Board::bishops, oppColor) | board->getPieceSet(Board::queens, oppColor);

   uint64_t oppSliderNorth = Board::northFill(oppRookLike, empty | king);
   uint64_t oppSliderSouth = Board::southFill(oppRookLike, empty | king);
   uint64_t oppSliderEast = Board::eastFill(oppRookLike, empty | king);
   uint64_t oppSliderWest = Board::westFill(oppRookLike, empty | king);
   uint64_t oppSliderNorthEast = Board::northEastFill(oppBishopLike, empty | king);
   uint64_t oppSliderNorthWest = Board::northWestFill(oppBishopLike, empty | king);
   uint64_t oppSliderSouthEast = Board::southEastFill(oppBishopLike, empty | king);
   uint64_t oppSliderSouthWest = Board::southWestFill(oppBishopLike, empty | king);

   uint64_t kingNorth = getPositiveRayAttacks(kingSquare, occupied, Board::north);
   uint64_t kingSouth = getNegativeRayAttacks(kingSquare, occupied, Board::south);
   uint64_t kingEast = getPositiveRayAttacks(kingSquare, occupied, Board::east);
   uint64_t kingWest = getNegativeRayAttacks(kingSquare, occupied, Board::west);
   uint64_t kingNorthEast = getPositiveRayAttacks(kingSquare, occupied, Board::northEast);
   uint64_t kingSouthWest = getNegativeRayAttacks(kingSquare, occupied, Board::southWest);
   uint64_t kingNorthWest = getPositiveRayAttacks(kingSquare, occupied, Board::northWest);
   uint64_t kingSouthEast = getNegativeRayAttacks(kingSquare, occupied, Board::southEast);
   
   uint64_t verInBetween = oppSliderNorth & kingSouth;
   verInBetween |= oppSliderSouth & kingNorth;
   populatePinMasks(pinMasks, indBuf, Board::ver, verInBetween);

   uint64_t horInBetween = oppSliderEast & kingWest;
   horInBetween |= oppSliderWest & kingEast;
   populatePinMasks(pinMasks, indBuf, Board::hor, horInBetween);

   uint64_t diagInBetween = oppSliderNorthEast & kingSouthWest;
   diagInBetween |= oppSliderSouthWest & kingNorthEast;
   populatePinMasks(pinMasks, indBuf, Board::diag, diagInBetween);

   uint64_t antiInBetween = oppSliderNorthWest & kingSouthEast;
   antiInBetween |= oppSliderSouthEast & kingNorthWest;
   populatePinMasks(pinMasks, indBuf, Board::anti, antiInBetween);

   uint64_t allInBetween = horInBetween | verInBetween | diagInBetween | antiInBetween;

   masks[Board::verMovable] = ~(allInBetween ^ verInBetween);
   masks[Board::diagMovable] = ~(allInBetween ^ diagInBetween);
   masks[Board::antiMovable] = ~(allInBetween ^ antiInBetween);
   masks[Board::pinned] = allInBetween;

   uint64_t kingSuperOrth = kingNorth | kingSouth | kingEast | kingWest;
   uint64_t kingSuperDiag = kingNorthEast | kingNorthWest | kingSouthEast | kingSouthWest;

   uint64_t oppPawnAttacks;
   if constexpr (color == Board::white) 
      oppPawnAttacks = Board::blackPawnTargets(oppPawns);
   else 
      oppPawnAttacks = Board::whitePawnTargets(oppPawns);
   uint64_t pawnCheckFrom = Tables::pawnAttackTable[color][kingSquare] & oppPawns;

   uint64_t oppAnyAttacks = oppPawnAttacks | oppSliderEast | oppSliderWest | oppSliderNorth | oppSliderSouth | oppSliderNorthEast | oppSliderSouthWest | oppSliderNorthWest 
         | oppSliderSouthEast | Board::knightAttackTargets(oppKnights) | Board::kingAttackTargets(oppKing);

   uint64_t blocks = allInBetween & empty;
   uint64_t checkFrom = (kingSuperOrth & oppRookLike) | (kingSuperDiag & oppBishopLike) 
         | pawnCheckFrom | (Tables::knightMoveTable[kingSquare] & oppKnights);

   int64_t nullIfCheck = Board::nullBoolMask(oppAnyAttacks & king);
   int64_t nullIfDoubleCheck = Board::nullBoolMask(checkFrom & (checkFrom - 1));
   uint64_t checkTo = checkFrom | blocks | nullIfCheck;
   uint64_t drawMask = Board::fullBoolMask(board->getHalfMoveClock() < 100);

   masks[Board::targetMask] = ~pieces & checkTo & nullIfDoubleCheck & drawMask;
   masks[Board::pawnTargetMask] = (masks[Board::targetMask] | (board->getEnPassantTarget(color) & Board::shift<up>(pawnCheckFrom))) & drawMask; //include en passant capture of pawn checker
   masks[Board::kingTargetMask] = ~(pieces | oppAnyAttacks) & drawMask;
   masks[Board::notInCheck] = nullIfCheck;
}

template<Board::PieceColor color>
void generate(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList) {
   std::array<uint16_t, NUM_SQUARES> indBuf;
   std::array<uint64_t, NUM_MASK_TYPES> masks;
   std::array<uint64_t, NUM_SQUARES> pinMasks;
   generateMasks<color>(board, masks, pinMasks, indBuf);

   appendSliderMoves<color>(board, moveList, indBuf, masks, pinMasks);
   appendKnightMoves<color>(board, moveList, indBuf, masks);
   appendPawnMoves<color>(board, moveList, indBuf, masks);
   appendKingMoves<color>(board, moveList, masks);
}

void MoveGen::getLegalMoves(Board* board, Board::PieceColor color, FixedVector<Move, MAX_LEGAL_MOVES>& moveList) {
   assert(Tables::initialized);
   color == Board::white ? generate<Board::white>(board, moveList) : generate<Board::black>(board, moveList); 
}