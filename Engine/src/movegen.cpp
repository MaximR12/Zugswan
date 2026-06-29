#include <array>
#include <unordered_map>
#include "movegen.hpp"

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
void serializeMoves(Board* board, std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, int direction) 
{
   if(!moveTargets[direction])
      return;

   uint64_t occupied = board->getOccupied();
   std::array<uint16_t, NUM_SQUARES> indices;
   int len = Board::serializeBitboard(moveTargets[direction], indices);

   for(int i = 0; i < len; ++i) {
      uint16_t to = indices[i];
      uint16_t from;
      if constexpr (type != MoveType::knight) 
         from = Board::serializeSingleBit(getRayAttacks(to, occupied, Board::getOppositeDirection(direction)) & occupied);
      else
         from = to + Board::getDirectionOffset(Board::getOppositeDirection(direction));
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
void appendSliderMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets,
      std::array<uint64_t, NUM_MASK_TYPES>& masks) 
{
   uint64_t targetMask = masks[Board::checkMask] & masks[Board::drawMask];
   uint64_t empty = board->getEmpty();
   uint64_t rookLike = board->getPieceSet(Board::rooks, color) | board->getPieceSet(Board::queens, color);
   uint64_t bishopLike = board->getPieceSet(Board::bishops, color) | board->getPieceSet(Board::queens, color);

   uint64_t verRookLike = rookLike & masks[Board::verPin];
   moveTargets[Board::north] = Board::northFill(verRookLike, empty) & targetMask;
   moveTargets[Board::south] = Board::southFill(verRookLike, empty) & targetMask;
   uint64_t horRookLike = rookLike & masks[Board::horPin];
   moveTargets[Board::east] = Board::eastFill(horRookLike, empty) & targetMask;
   moveTargets[Board::west] = Board::westFill(horRookLike, empty) & targetMask;
   uint64_t diagBishopLike = bishopLike & masks[Board::diagPin];
   moveTargets[Board::northEast] = Board::northEastFill(diagBishopLike, empty) & targetMask;
   moveTargets[Board::southWest] = Board::southWestFill(diagBishopLike, empty) & targetMask;
   uint64_t antiBishopLike = bishopLike & masks[Board::antiPin];
   moveTargets[Board::northWest] = Board::northWestFill(antiBishopLike, empty) & targetMask;
   moveTargets[Board::southEast] = Board::southEastFill(antiBishopLike, empty) & targetMask;

   for(int dir = 0; dir < NUM_SLIDER_DIRECTIONS; ++dir) //loop slider directions
      if(moveTargets[dir]) serializeMoves<color, MoveType::slider>(board, moveTargets, moveList, dir); 
}

template<Board::PieceColor color>
void appendKnightMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets,
      std::array<uint64_t, NUM_MASK_TYPES>& masks) 
{
   uint64_t targetMask = masks[Board::checkMask] & masks[Board::drawMask];
   uint64_t knights = board->getPieceSet(Board::knights, color);
   knights &= masks[Board::allPin];

   moveTargets[Board::northNorthEast] = Board::shift<Board::northNorthEast>(knights) & targetMask; 
   moveTargets[Board::northEastEast] = Board::shift<Board::northEastEast>(knights) & targetMask;
   moveTargets[Board::northNorthWest] = Board::shift<Board::northNorthWest>(knights) & targetMask;
   moveTargets[Board::northWestWest] = Board::shift<Board::northWestWest>(knights) & targetMask;
   moveTargets[Board::southSouthEast] = Board::shift<Board::southSouthEast>(knights) & targetMask; 
   moveTargets[Board::southEastEast] = Board::shift<Board::southEastEast>(knights) & targetMask;
   moveTargets[Board::southSouthWest] = Board::shift<Board::southSouthWest>(knights) & targetMask;
   moveTargets[Board::southWestWest] = Board::shift<Board::southWestWest>(knights) & targetMask;

   for(int dir = Board::northNorthEast; dir < NUM_TOTAL_DIRECTIONS; ++dir) //loop knight directions
      if(moveTargets[dir]) serializeMoves<color, MoveType::knight>(board, moveTargets, moveList, dir); 
}

template<Board::PieceColor color>
void appendPawnMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets,
      std::array<uint64_t, NUM_MASK_TYPES>& masks) 
{
   constexpr Board::MaskTypes leftDiag = color == Board::white ? Board::antiPin : Board::diagPin;
   constexpr Board::MaskTypes rightDiag = color == Board::white ? Board::diagPin : Board::antiPin;
   constexpr Board::Directions up = color == Board::white ? Board::north : Board::south;
   constexpr Board::Directions upRight = color == Board::white ? Board::northEast : Board::southEast;
   constexpr Board::Directions upLeft = color == Board::white ? Board::northWest : Board::southWest;
   constexpr Board::PieceColor oppColor = color == Board::white ? Board::black : Board::white;
   constexpr uint64_t pawnRank = color == Board::white ? RANK_2 : RANK_7;

   uint64_t pawns = board->getPieceSet(Board::pawns, color);
   uint64_t oppPieces = board->getPieceSet(Board::all, oppColor);
   uint64_t epAttackTarget = board->getEnPassantTarget(color);
   uint64_t empty = board->getEmpty();
   uint64_t targetMask = masks[Board::pawnCheckMask] & masks[Board::drawMask]; 

   //pushes
   uint64_t pushable = pawns & masks[Board::verPin];
   uint64_t singlePushTargets = Board::shift<up>(pushable) & empty & targetMask;
   moveTargets[up] = singlePushTargets & NOT_LAST_RANK;
   uint64_t doublePushTargets = Board::shift<up>(pushable & pawnRank) & empty;
   moveTargets[up] |= Board::shift<up>(doublePushTargets) & empty & targetMask;

   //attacks
   uint64_t leftAttacks = Board::shift<upLeft>(pawns & masks[leftDiag]) & (oppPieces | epAttackTarget) & targetMask;
   moveTargets[upLeft] = leftAttacks & NOT_LAST_RANK;
   uint64_t rightAttacks = Board::shift<upRight>(pawns & masks[rightDiag]) & (oppPieces | epAttackTarget) & targetMask;
   moveTargets[upRight] = rightAttacks & NOT_LAST_RANK;

   if(moveTargets[up]) serializeMoves<color, MoveType::pawn>(board, moveTargets, moveList, up);
   if(moveTargets[upLeft]) serializeMoves<color, MoveType::pawn>(board, moveTargets, moveList, upLeft);
   if(moveTargets[upRight]) serializeMoves<color, MoveType::pawn>(board, moveTargets, moveList, upRight);

   //promotions
   moveTargets[up] = singlePushTargets & LAST_RANK;
   moveTargets[upLeft] = leftAttacks & LAST_RANK;
   moveTargets[upRight] = rightAttacks & LAST_RANK;

   if(moveTargets[up]) serializeMoves<color, MoveType::promotion>(board, moveTargets, moveList, up);
   if(moveTargets[upLeft]) serializeMoves<color, MoveType::promotion>(board, moveTargets, moveList, upLeft);
   if(moveTargets[upRight]) serializeMoves<color, MoveType::promotion>(board, moveTargets, moveList, upRight);
}

template<Board::PieceColor color>
void appendKingMoves(Board* board, FixedVector<Move, MAX_LEGAL_MOVES>& moveList, std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets,
      std::array<uint64_t, NUM_MASK_TYPES>& masks) 
{
   uint64_t targetMask = masks[Board::kingMask] & masks[Board::drawMask];
   uint64_t king = board->getPieceSet(Board::king, color);
   
   moveTargets[Board::north] = Board::shift<Board::north>(king) & targetMask;
   moveTargets[Board::south] = Board::shift<Board::south>(king) & targetMask;
   moveTargets[Board::east] = Board::shift<Board::east>(king) & targetMask;
   moveTargets[Board::west] = Board::shift<Board::west>(king) & targetMask;
   moveTargets[Board::northEast] = Board::shift<Board::northEast>(king) & targetMask;
   moveTargets[Board::northWest] = Board::shift<Board::northWest>(king) & targetMask;
   moveTargets[Board::southEast] = Board::shift<Board::southEast>(king) & targetMask;
   moveTargets[Board::southWest] = Board::shift<Board::southWest>(king) & targetMask;

   bool kingCastleRights = board->getKingCastleRights(color);
   bool queenCastleRights = board->getQueenCastleRights(color);
   uint64_t rooks = board->getPieceSet(Board::rooks, color);
   uint64_t occupied = board->getOccupied();
   uint64_t empty = board->getEmpty();
   uint64_t notInCheck = masks[Board::notInCheck];

   //castle moves
   targetMask &= empty;
   uint64_t kingCastleMask = Board::fullBoolMask(kingCastleRights), queenCastleMask = Board::fullBoolMask(queenCastleRights);
   uint64_t eastOne = Board::shift<Board::east>(king) & targetMask;
   moveTargets[Board::east] |= Board::shift<Board::east>(eastOne) & targetMask & kingCastleMask & Board::fullBoolMask((king << 3) & rooks) & notInCheck;
   uint64_t westOne = Board::shift<Board::west>(king) & targetMask;
   uint64_t westTwo = Board::shift<Board::west>(westOne) & targetMask;
   moveTargets[Board::west] |= westTwo & Board::nullBoolMask(Board::shift<Board::west>(westTwo) & occupied) & queenCastleMask & Board::fullBoolMask((king >> 4) & rooks) & notInCheck; //queen castle includes occupency check of square west of queen rook

   for(int dir = 0; dir < NUM_SLIDER_DIRECTIONS; ++dir) //loop slider directions
      if(moveTargets[dir]) serializeMoves<color, MoveType::king>(board, moveTargets, moveList, dir); 
}

//populates mask array
template<Board::PieceColor color>
void generateLegalityMasks(Board* board, Tables* tables, std::array<uint64_t, NUM_MASK_TYPES>& masks) {
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
   uint64_t horInBetween = oppSliderEast & kingWest;
   horInBetween |= oppSliderWest & kingEast;
   uint64_t diagInBetween = oppSliderNorthEast & kingSouthWest;
   diagInBetween |= oppSliderSouthWest & kingNorthEast;
   uint64_t antiInBetween = oppSliderNorthWest & kingSouthEast;
   antiInBetween |= oppSliderSouthEast & kingNorthWest;
   uint64_t allInBetween = horInBetween | verInBetween | diagInBetween | antiInBetween;

   masks[Board::allPin] = ~allInBetween;
   masks[Board::verPin] = ~(allInBetween ^ verInBetween);
   masks[Board::horPin] = ~(allInBetween ^ horInBetween);
   masks[Board::diagPin] = ~(allInBetween ^ diagInBetween);
   masks[Board::antiPin] = ~(allInBetween ^ antiInBetween);

   uint64_t kingSuperOrth = kingNorth | kingSouth | kingEast | kingWest;
   uint64_t kingSuperDiag = kingNorthEast | kingNorthWest | kingSouthEast | kingSouthWest;

   uint64_t oppPawnAttacks;
   if constexpr (color == Board::white) 
      oppPawnAttacks = Board::blackPawnTargets(oppPawns);
   else 
      oppPawnAttacks = Board::whitePawnTargets(oppPawns);
   uint64_t pawnCheckFrom = tables->getPawnSquareAttacks(kingSquare, color) & oppPawns;

   uint64_t oppAnyAttacks = oppPawnAttacks | oppSliderEast | oppSliderWest | oppSliderNorth | oppSliderSouth | oppSliderNorthEast | oppSliderSouthWest | oppSliderNorthWest 
         | oppSliderSouthEast | Board::knightAttackTargets(oppKnights) | Board::kingAttackTargets(oppKing);

   uint64_t blocks = allInBetween & empty;
   uint64_t checkFrom = (kingSuperOrth & oppRookLike) | (kingSuperDiag & oppBishopLike) 
         | pawnCheckFrom | (tables->getKnightMoves(kingSquare) & oppKnights);

   int64_t nullIfCheck = Board::nullBoolMask(oppAnyAttacks & king);
   int64_t nullIfDoubleCheck = Board::nullBoolMask(checkFrom & (checkFrom - 1));
   uint64_t checkTo = checkFrom | blocks | nullIfCheck;

   masks[Board::drawMask] = Board::fullBoolMask(board->getHalfMoveClock() < 100);
   masks[Board::checkMask] = ~pieces & checkTo & nullIfDoubleCheck;
   masks[Board::pawnCheckMask] = (masks[Board::checkMask] | (board->getEnPassantTarget(color) & Board::shift<up>(pawnCheckFrom))); //include en passant capture of pawn checker
   masks[Board::kingMask] = ~(pieces | oppAnyAttacks);
   masks[Board::notInCheck] = nullIfCheck;
}

//Dirgolem move generation, generate 16 move target bitboards for each direction, then serialize into move objects
template<Board::PieceColor color>
void generate(Board* board, Tables* tables, FixedVector<Move, MAX_LEGAL_MOVES>& moveList) {
   std::array<uint64_t, NUM_TOTAL_DIRECTIONS> moveTargets;
   std::array<uint64_t, NUM_MASK_TYPES> masks;
   generateLegalityMasks<color>(board, tables, masks);

   appendSliderMoves<color>(board, moveList, moveTargets, masks);
   appendKnightMoves<color>(board, moveList, moveTargets, masks);
   appendPawnMoves<color>(board, moveList, moveTargets, masks);
   appendKingMoves<color>(board, moveList, moveTargets, masks);
}

void MoveGen::getLegalMoves(Board::PieceColor color, FixedVector<Move, MAX_LEGAL_MOVES>& moveList) const {
   color == Board::white ? generate<Board::white>(m_board, m_tables, moveList) : generate<Board::black>(m_board, m_tables, moveList); 
}