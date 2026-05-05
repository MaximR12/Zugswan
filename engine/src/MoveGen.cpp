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

uint64_t getHorSliderAttacks(uint64_t rookLike, uint64_t king, uint64_t empty) {
   return Board::westFill(rookLike, empty | king) | Board::eastFill(rookLike, empty | king);
}

uint64_t getVerSliderAttacks(uint64_t rookLike, uint64_t king, uint64_t empty) {
   return Board::northFill(rookLike, empty | king) | Board::southFill(rookLike, empty | king);
}

uint64_t getDiagSliderAttacks(uint64_t bishopLike, uint64_t king, uint64_t empty) {
   return Board::northEastFill(bishopLike, empty | king) | Board::southWestFill(bishopLike, empty | king);
}

uint64_t getAntiSliderAttacks(uint64_t bishopLike, uint64_t king, uint64_t empty) {
   return Board::northWestFill(bishopLike, empty | king) | Board::southEastFill(bishopLike, empty | king);
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

void appendPawnMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, uint64_t pawns, Board::Directions pawnDir, uint64_t oppPieces, uint64_t epAttackTargets,
      uint64_t verInBetween, uint64_t diagInBetween, uint64_t antiInBetween, uint64_t allInBetween, uint64_t empty, uint64_t checkMask) 
{
   uint64_t pinSafe = pawns & ~(allInBetween ^ antiInBetween);
   uint64_t singlePushTargets = Board::pawnShift(pinSafe, pawnDir) & empty;
   moveTargets[pawnDir] |= singlePushTargets & checkMask;

   uint64_t doublePushTargets = Board::pawnShift(pinSafe & PAWN_RANK, pawnDir) & empty;
   moveTargets[pawnDir] |= Board::pawnShift(doublePushTargets, pawnDir) & empty & checkMask;

   uint64_t pawnAttackTargets = Board::pawnAttackTargetsSafe(pawns, pawnDir, diagInBetween, antiInBetween, allInBetween) & (oppPieces | epAttackTargets);
   moveTargets[Board::northEast] |= pawnAttackTargets & Board::shiftNorthEast(pawns) & checkMask;
   moveTargets[Board::northWest] |= pawnAttackTargets & Board::shiftNorthWest(pawns) & checkMask;
   moveTargets[Board::southEast] |= pawnAttackTargets & Board::shiftSouthEast(pawns) & checkMask;
   moveTargets[Board::southWest] |= pawnAttackTargets & Board::shiftSouthWest(pawns) & checkMask;
}

void appendKingMoves(std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, uint64_t pieces, uint64_t king, uint64_t oppAnyAttacks) {
   uint64_t targetMask = ~(pieces | oppAnyAttacks);
   moveTargets[Board::north] |= Board::shiftNorth(king) & targetMask;
   moveTargets[Board::south] |= Board::shiftSouth(king) & targetMask;
   moveTargets[Board::east] |= Board::shiftEast(king) & targetMask;
   moveTargets[Board::west] |= Board::shiftWest(king) & targetMask;
   moveTargets[Board::northEast] |= Board::shiftNorthEast(king) & targetMask;
   moveTargets[Board::northWest] |= Board::shiftNorthWest(king) & targetMask;
   moveTargets[Board::southEast] |= Board::shiftSouthEast(king) & targetMask;
   moveTargets[Board::southWest] |= Board::shiftSouthWest(king) & targetMask;
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

uint16_t getFlag(uint16_t from, uint16_t to, uint64_t pawns, uint64_t oppPieces, uint64_t epTargets) {
   uint64_t toBB = 1ULL << to, fromBB = 1ULL << from;
   uint16_t flag = QUIET_MOVE;

   int64_t nullIfCapture = ((int64_t)((1ULL << to) & oppPieces) - 1) >> 63;
   flag |= CAPTURE & ~nullIfCapture;

   // uint64_t fromIfPawn = fromBB & pawns;
   // int64_t pawnMask = Board::fullBoolMask(fromIfPawn);

   // uint64_t nonZeroIfDoublePush = ((toBB << 16) & fromBB) | ((toBB >> 16) & fromBB);
   // flag |= DOUBLE_PAWN_PUSH & Board::fullBoolMask(nonZeroIfDoublePush) & pawnMask;
   // flag |= EP_CAPTURE & Board::fullBoolMask(to & epTargets) & pawnMask;

   return flag;
}

void serializeSliderMoves(std::vector<Move>& moves, std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS> targetInds, 
      std::array<uint16_t, NUM_TOTAL_DIRECTIONS> targetIndLengths, uint64_t pawns, uint64_t oppPieces, uint64_t epTargets, uint64_t occupied) 
{
   for(int dir = 0; dir < NUM_SLIDER_DIRECTIONS; ++dir) { 
      uint16_t len = targetIndLengths[dir];
      for(int i = 0; i < len; ++i) {
         uint16_t to = targetInds[dir][i];
         uint16_t from = Board::serializeSingleBit(getRayAttacks(to, occupied, Board::getOppositeDirection(dir)) & occupied);
         uint16_t flag = getFlag(from, to, pawns, oppPieces, epTargets);

         moves.emplace_back(flag, from, to);
      }
   } 
}

void serializeMoves(std::vector<Move>& moves, std::array<uint64_t, NUM_TOTAL_DIRECTIONS>& moveTargets, 
      uint64_t pawns, uint64_t oppPieces, uint64_t epTargets, uint64_t occupied) 
{
   std::array<uint16_t, NUM_TOTAL_DIRECTIONS> targetIndLengths; 
   std::array<std::array<uint16_t, NUM_SQUARES>, NUM_TOTAL_DIRECTIONS> targetInds;
   for(int dir = 0; dir < NUM_TOTAL_DIRECTIONS; ++dir) 
      targetIndLengths[dir] = Board::serializeBitboard(moveTargets[dir], targetInds[dir]);

   serializeKnightMoves(moves, targetInds, targetIndLengths, oppPieces);
   serializeSliderMoves(moves, targetInds, targetIndLengths, pawns, oppPieces, epTargets, occupied);
}

//Dirgolem move generation, generate 16 move target bitboards for each direction, then serialize into moves
std::vector<Move> MoveGen::getLegalMoves(Turn turn) {
   std::array<uint64_t, NUM_TOTAL_DIRECTIONS> moveTargets;

   Board::PieceType pieceColor;
   Board::Directions pawnDir, oppPawnDir;
   uint64_t pieces, king, pawns, rookLike, bishopLike, knights, oppPieces, epAttackTargets, oppKing, oppPawns, oppKnights, oppRookLike, oppBishopLike;
   if(turn == Turn::WHITE) {
      pieceColor = Board::whitePieces;
      pawnDir = Board::north, oppPawnDir = Board::south;
      pieces = m_board->getPieceSet(Board::whitePieces);
      king = m_board->getPieceSet(Board::whiteKing);
      pawns = m_board->getPieceSet(Board::whitePawns);
      rookLike = m_board->getPieceSet(Board::whiteRooks) | m_board->getPieceSet(Board::whiteQueens);
      bishopLike = m_board->getPieceSet(Board::whiteBishops) | m_board->getPieceSet(Board::whiteQueens);
      knights = m_board->getPieceSet(Board::whiteKnights);
      oppPieces = m_board->getPieceSet(Board::blackPieces);
      epAttackTargets = m_board->getWhiteEpTargets();
      oppKing = m_board->getPieceSet(Board::blackKing);
      oppPawns = m_board->getPieceSet(Board::blackPawns);
      oppKnights = m_board->getPieceSet(Board::blackKnights);
      oppRookLike = m_board->getPieceSet(Board::blackRooks) | m_board->getPieceSet(Board::blackQueens);
      oppBishopLike = m_board->getPieceSet(Board::blackBishops) | m_board->getPieceSet(Board::blackQueens);
   } else {
      pieceColor = Board::blackPieces;
      pawnDir = Board::south, oppPawnDir = Board::north;
      pieces = m_board->getPieceSet(Board::blackPieces);
      king = m_board->getPieceSet(Board::blackKing);
      pawns = m_board->getPieceSet(Board::blackPawns);
      rookLike = m_board->getPieceSet(Board::blackRooks) | m_board->getPieceSet(Board::blackQueens);
      bishopLike = m_board->getPieceSet(Board::PieceType::blackBishops) | m_board->getPieceSet(Board::blackQueens);
      knights = m_board->getPieceSet(Board::blackKnights);
      oppPieces = m_board->getPieceSet(Board::whitePieces);
      epAttackTargets = m_board->getBlackEpTargets();
      oppKing = m_board->getPieceSet(Board::whiteKing);
      oppPawns = m_board->getPieceSet(Board::whitePawns);
      oppKnights = m_board->getPieceSet(Board::whiteKnights);
      oppRookLike = m_board->getPieceSet(Board::whiteRooks) | m_board->getPieceSet(Board::whiteQueens);
      oppBishopLike = m_board->getPieceSet(Board::whiteBishops) | m_board->getPieceSet(Board::whiteQueens);
   }

   uint16_t kingInd = Board::serializeSingleBit(king);
   uint64_t empty = m_board->getEmpty();
   uint64_t occupied = m_board->getOccupied();

   uint64_t horSliderAttacks = getHorSliderAttacks(oppRookLike, king, empty);
   uint64_t verSliderAttacks = getVerSliderAttacks(oppRookLike, king, empty);
   uint64_t diagSliderAttacks = getDiagSliderAttacks(oppBishopLike, king, empty);
   uint64_t antiSliderAttacks = getAntiSliderAttacks(oppBishopLike, king, empty);

   uint64_t oppSliderNorth = Board::northFill(oppRookLike, empty & king);
   uint64_t oppSliderSouth = Board::southFill(oppRookLike, empty & king);
   uint64_t oppSliderEast = Board::eastFill(oppRookLike, empty & king);
   uint64_t oppSliderWest = Board::westFill(oppRookLike, empty & king);
   uint64_t oppSliderNorthEast = Board::northEastFill(oppBishopLike, empty & king);
   uint64_t oppSliderNorthWest = Board::northWestFill(oppBishopLike, empty & king);
   uint64_t oppSliderSouthEast = Board::southEastFill(oppBishopLike, empty & king);
   uint64_t oppSliderSouthWest = Board::southWestFill(oppBishopLike, empty & king);

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

   uint64_t oppPawnAttacks = Board::pawnAttackTargets(oppPawns, oppPawnDir);
   uint64_t pawnCheckFrom = m_board->getPawnSquareAttacks(kingInd, pieceColor) & oppPawns;

   uint64_t oppAnyAttacks = oppPawnAttacks | horSliderAttacks | verSliderAttacks | diagSliderAttacks | antiSliderAttacks 
         | Board::knightAttackTargets(oppKnights) | Board::kingAttackTargets(oppKing);

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
   appendKnightMoves(moveTargets, knights, allInBetween, checkMask);
   appendPawnMoves(moveTargets, pawns, pawnDir, oppPieces, epAttackTargets, verInBetween, diagInBetween, antiInBetween, allInBetween, empty, checkMask);
   appendKingMoves(moveTargets, pieces, king, oppAnyAttacks);

   std::vector<Move> moves;
   moves.reserve(MAX_LEGAL_MOVES);
   serializeMoves(moves, moveTargets, pawns, oppPieces, epAttackTargets, m_board->getOccupied());

   return moves;
}