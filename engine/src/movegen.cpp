#include <array>
#include <unordered_map>
#include "movegen.hpp"

enum class MoveType {
   pawn, promotion, knight, bishop, rook, queen, king
};

struct Masks {
   uint64_t pinned;
   uint64_t verMovable;
   uint64_t diagMovable;
   uint64_t antiMovable;
   uint64_t targetMask;
   uint64_t pawnTargetMask;
   uint64_t kingTargetMask;
   uint64_t notInCheck;
};

template<Board::PieceColor color, MoveType type>
int16_t scoreMove(Board& board, Move move, uint16_t ply) {
   if(Move::isCapture(move.getFlag())) {      
      constexpr Board::PieceColor oppColor = color == Board::white ? Board::black : Board::white;
      uint16_t to = move.getTo();

      if constexpr (type == MoveType::king) 
         return GOOD_CAPTURE_BASE + Board::getPieceValue(board.getPieceType(to, oppColor));

      constexpr Board::PieceType attackerType 
         = type == MoveType::pawn ? Board::pawns 
         : type == MoveType::knight ? Board::knights 
         : type == MoveType::bishop ? Board::bishops
         : type == MoveType::rook ? Board::rooks
         : Board::queens;
      
      int16_t staticCaptureEval = move.getFlag() != EP_CAPTURE ? board.staticCaptureEvaluation(attackerType, oppColor, to) : 0;
      if(staticCaptureEval >= 0)
         return GOOD_CAPTURE_BASE + staticCaptureEval;
      else {
         int16_t staticExchangeEval = board.staticExchangeEvaluation(move);
         return staticExchangeEval >= 0 ? GOOD_CAPTURE_BASE + staticExchangeEval : BAD_CAPTURE_BASE + staticExchangeEval;
      }
   }

   return QUIET_BASE + KILLER_BASE * Tables::isKiller(move, ply) + Tables::historyScore(board.getTurn(), move.getFrom(), move.getTo());
}

template<Board::PieceColor color, MoveType type>
uint16_t getFlag(Board& board, uint16_t from, uint16_t to) {
   constexpr Board::PieceColor oppColor = color == Board::white ? Board::black : Board::white;

   uint64_t oppPieces = board.getPieceSet(Board::all, oppColor);
   uint64_t toBB = 1ULL << to, fromBB = 1ULL << from;
   
   uint16_t flag = QUIET_MOVE;
   int64_t captureMask = Board::fullBoolMask(toBB & oppPieces);
   flag |= CAPTURE & captureMask;

   if constexpr (type == MoveType::pawn) {
      uint64_t pawns = board.getPieceSet(Board::pawns, color);
      uint64_t epTargets = board.getEnPassantTarget(color);
      uint64_t fromIfPawn = fromBB & pawns;
      int64_t pawnMask = Board::fullBoolMask(fromIfPawn);

      uint64_t nonZeroIfDoublePush = ((toBB << 16) & fromBB) | ((toBB >> 16) & fromBB);
      flag |= DOUBLE_PAWN_PUSH & Board::fullBoolMask(nonZeroIfDoublePush) & pawnMask;
      flag |= EP_CAPTURE & Board::fullBoolMask(toBB & epTargets) & pawnMask;
   }
   
   if constexpr (type == MoveType::king) {
      uint64_t king = board.getPieceSet(Board::king, color);

      uint64_t nonZeroIfKingCastle = (toBB >> 2) & king & fromBB;
      uint64_t nonZeroIfQueenCastle = (toBB << 2) & king & fromBB;
      flag |= KING_CASTLE & Board::fullBoolMask(nonZeroIfKingCastle);
      flag |= QUEEN_CASTLE & Board::fullBoolMask(nonZeroIfQueenCastle);
   }

   return flag;
}

template<Board::PieceColor color, MoveType type>
void serializeMoves(Board& board, MoveList& moveList, uint64_t targets, int16_t from, uint16_t ply) 
{
   int16_t fromOffset;
   if constexpr (type == MoveType::pawn || type == MoveType::promotion)
      fromOffset = from; //from acts as offset for pawn moves

   uint64_t occupied = board.getOccupied();
   std::array<uint16_t, NUM_SQUARES> indices;
   int len = Board::serializeBitboard(targets, indices);

   for(int i = 0; i < len; ++i) {
      uint16_t to = indices[i];
      if constexpr (type == MoveType::pawn || type == MoveType::promotion) 
         from = to + fromOffset; 
      uint16_t flag = getFlag<color, type>(board, from, to);

      if constexpr (type != MoveType::promotion) {
         Move move = Move(flag, from, to);
         moveList.push_back(move);
         moveList.back().m_score = scoreMove<color, type>(board, move, ply);
      } else {
         for(uint16_t promoType = KNIGHT_PROMOTION; promoType <= QUEEN_PROMOTION; ++promoType) {
            Move move = Move(flag | promoType, from, to);
            moveList.push_back(move);
            moveList.back().m_score = scoreMove<color, type>(board, move, ply);
         }
      }
   }
}

template<Board::PieceColor color, MoveType type>
void appendSliderMoves(Board& board, MoveList& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf,
      std::array<uint64_t, NUM_SQUARES>& pinMasks, Masks& masks, uint16_t ply)
{
   assert(type == MoveType::bishop || type == MoveType::rook || type == MoveType::queen);

   uint64_t occupied = board.getOccupied();
   uint64_t pieceSet;
   if constexpr (type == MoveType::bishop)
      pieceSet = board.getPieceSet(Board::bishops, color);
   else if constexpr (type == MoveType::rook)
      pieceSet = board.getPieceSet(Board::rooks, color);
   else
      pieceSet = board.getPieceSet(Board::queens, color);

   uint16_t numPieces = Board::serializeBitboard(pieceSet, indBuf);
   for(int i = 0; i < numPieces; ++i) {
      uint16_t square = indBuf[i];
      uint64_t squareBB = 1ULL<<square;
      uint64_t psuedoLegal; 
      if constexpr (type == MoveType::bishop)
         psuedoLegal = Tables::bishopAttacks(square, occupied);
      else if constexpr (type == MoveType::rook)
         psuedoLegal = Tables::rookAttacks(square, occupied);
      else
         psuedoLegal = Tables::bishopAttacks(square, occupied) | Tables::rookAttacks(square, occupied);

      uint64_t pinMask = UNIVERSE;
      if(squareBB & masks.pinned)
         pinMask = pinMasks[square];

      uint64_t legal = psuedoLegal & pinMask & masks.targetMask;
      if(legal) serializeMoves<color, type>(board, moveList, legal, square, ply);
   }
}

template<Board::PieceColor color>
void appendKnightMoves(Board& board, MoveList& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, Masks& masks, uint16_t ply) {
   uint64_t knights = board.getPieceSet(Board::knights, color);
   knights &= ~masks.pinned;

   uint64_t numKnights = Board::serializeBitboard(knights, indBuf);
   for(int i = 0; i < numKnights; ++i) {
      uint16_t square = indBuf[i];
      uint64_t psuedoLegal = Tables::knightMoves(square);
      
      uint64_t legal = psuedoLegal & masks.targetMask;
      if(legal) serializeMoves<color, MoveType::knight>(board, moveList, legal, square, ply);
   }
}

template<Board::PieceColor color>
void appendPawnMoves(Board& board, MoveList& moveList, std::array<uint16_t, NUM_SQUARES>& indBuf, Masks& masks, uint16_t ply) {   
   constexpr Board::PieceColor oppColor = color == Board::white ? Board::black : Board::white;
   constexpr uint64_t pawnRank = color == Board::white ? RANK_2 : RANK_7;

   constexpr Board::Directions up = color == Board::white ? Board::north : Board::south;
   constexpr Board::Directions upRight = color == Board::white ? Board::northEast : Board::southEast;
   constexpr Board::Directions upLeft = color == Board::white ? Board::northWest : Board::southWest;

   constexpr int16_t downOffset = color == Board::white ? southOffset : northOffset;
   constexpr int16_t downLeftOffset = color == Board::white ? southWestOffset : northWestOffset;
   constexpr int16_t downRightOffset = color == Board::white ? southEastOffset : northEastOffset;

   uint64_t leftDiagMask, rightDiagMask;
   if constexpr (color == Board::white)
      leftDiagMask = masks.antiMovable, rightDiagMask = masks.diagMovable;
   else
      leftDiagMask = masks.diagMovable, rightDiagMask = masks.antiMovable;

   uint64_t pawns = board.getPieceSet(Board::pawns, color);
   uint64_t oppPieces = board.getPieceSet(Board::all, oppColor);
   uint64_t epAttackTarget = board.getEnPassantTarget(color);
   uint64_t empty = board.getEmpty();
   uint64_t targetMask = masks.pawnTargetMask; 

   uint64_t pushable = pawns & masks.verMovable;
   uint64_t singlePushTargets = Board::shift<up>(pushable) & empty & targetMask;
   uint64_t doublePushTargets = Board::shift<up>(pushable & pawnRank) & empty;
   doublePushTargets = Board::shift<up>(doublePushTargets) & empty & targetMask;
   uint64_t leftAttacks = Board::shift<upLeft>(pawns & leftDiagMask) & (oppPieces | epAttackTarget) & targetMask;
   uint64_t rightAttacks = Board::shift<upRight>(pawns & rightDiagMask) & (oppPieces | epAttackTarget) & targetMask;
   
   if(singlePushTargets & NOT_LAST_RANK) serializeMoves<color, MoveType::pawn>(board, moveList, singlePushTargets & NOT_LAST_RANK, downOffset, ply);
   if(doublePushTargets) serializeMoves<color, MoveType::pawn>(board, moveList, doublePushTargets, downOffset*2, ply);
   if(rightAttacks & NOT_LAST_RANK) serializeMoves<color, MoveType::pawn>(board, moveList, rightAttacks & NOT_LAST_RANK, downLeftOffset, ply);
   if(leftAttacks & NOT_LAST_RANK) serializeMoves<color, MoveType::pawn>(board, moveList, leftAttacks & NOT_LAST_RANK, downRightOffset, ply);

   if(singlePushTargets & LAST_RANK) serializeMoves<color, MoveType::promotion>(board, moveList, singlePushTargets & LAST_RANK, downOffset, ply);
   if(rightAttacks & LAST_RANK) serializeMoves<color, MoveType::promotion>(board, moveList, rightAttacks & LAST_RANK, downLeftOffset, ply);
   if(leftAttacks & LAST_RANK) serializeMoves<color, MoveType::promotion>(board, moveList, leftAttacks & LAST_RANK, downRightOffset, ply);
}

template<Board::PieceColor color>
void appendKingMoves(Board& board, MoveList& moveList, Masks& masks, uint16_t ply) {   
   uint64_t targetMask = masks.kingTargetMask;
   uint64_t king = board.getPieceSet(Board::king, color);
   uint16_t kingSquare = Board::serializeSingleBit(king);
   
   uint64_t targets = Tables::kingMoves(kingSquare) & targetMask;
   if(targets) serializeMoves<color, MoveType::king>(board, moveList, targets, kingSquare, ply);

   bool kingCastleRights = board.getKingCastleRights(color);
   bool queenCastleRights = board.getQueenCastleRights(color);

   if(!kingCastleRights && !queenCastleRights)
      return;

   uint64_t rooks = board.getPieceSet(Board::rooks, color);
   uint64_t occupied = board.getOccupied();
   uint64_t empty = board.getEmpty();
   uint64_t notInCheck = masks.notInCheck;

   targetMask &= empty;
   uint64_t kingCastleMask = Board::fullBoolMask(kingCastleRights), queenCastleMask = Board::fullBoolMask(queenCastleRights);
   uint64_t eastOne = Board::shift<Board::east>(king) & targetMask;
   uint64_t leftCastle = Board::shift<Board::east>(eastOne) & targetMask & kingCastleMask & Board::fullBoolMask((king << 3) & rooks) & notInCheck;
   uint64_t westOne = Board::shift<Board::west>(king) & targetMask;
   uint64_t westTwo = Board::shift<Board::west>(westOne) & targetMask;
   uint64_t rightCastle = westTwo & Board::nullBoolMask(Board::shift<Board::west>(westTwo) & occupied) & queenCastleMask & Board::fullBoolMask((king >> 4) & rooks) & notInCheck; //queen castle includes occupency check of square west of queen rook

   uint64_t castleTargets = leftCastle | rightCastle;
   if(castleTargets) serializeMoves<color, MoveType::king>(board, moveList, castleTargets, kingSquare, ply);
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
void generateMasks(Board& board, Masks& masks, std::array<uint64_t, NUM_SQUARES>& pinMasks, std::array<uint16_t, NUM_SQUARES>& indBuf) {
   constexpr Board::PieceColor oppColor = color == Board::white ? Board::black : Board::white;
   constexpr Board::Directions up = color == Board::white ? Board::north : Board::south;

   uint64_t empty = board.getEmpty();
   uint64_t occupied = board.getOccupied();
   uint64_t pieces = board.getPieceSet(Board::all, color);
   uint64_t king = board.getPieceSet(Board::king, color);
   uint16_t kingSquare = Board::serializeSingleBit(king);
   uint64_t oppKing = board.getPieceSet(Board::king, oppColor);
   uint64_t oppPawns = board.getPieceSet(Board::pawns, oppColor);
   uint64_t oppKnights = board.getPieceSet(Board::knights, oppColor);
   uint64_t oppRookLike = board.getPieceSet(Board::rooks, oppColor) | board.getPieceSet(Board::queens, oppColor);
   uint64_t oppBishopLike = board.getPieceSet(Board::bishops, oppColor) | board.getPieceSet(Board::queens, oppColor);

   uint64_t oppSliderNorth = Board::northFill(oppRookLike, empty | king);
   uint64_t oppSliderSouth = Board::southFill(oppRookLike, empty | king);
   uint64_t oppSliderEast = Board::eastFill(oppRookLike, empty | king);
   uint64_t oppSliderWest = Board::westFill(oppRookLike, empty | king);
   uint64_t oppSliderNorthEast = Board::northEastFill(oppBishopLike, empty | king);
   uint64_t oppSliderNorthWest = Board::northWestFill(oppBishopLike, empty | king);
   uint64_t oppSliderSouthEast = Board::southEastFill(oppBishopLike, empty | king);
   uint64_t oppSliderSouthWest = Board::southWestFill(oppBishopLike, empty | king);

   uint64_t kingNorth = Board::getPositiveRayAttacks(kingSquare, occupied, Board::north);
   uint64_t kingSouth = Board::getNegativeRayAttacks(kingSquare, occupied, Board::south);
   uint64_t kingEast = Board::getPositiveRayAttacks(kingSquare, occupied, Board::east);
   uint64_t kingWest = Board::getNegativeRayAttacks(kingSquare, occupied, Board::west);
   uint64_t kingNorthEast = Board::getPositiveRayAttacks(kingSquare, occupied, Board::northEast);
   uint64_t kingSouthWest = Board::getNegativeRayAttacks(kingSquare, occupied, Board::southWest);
   uint64_t kingNorthWest = Board::getPositiveRayAttacks(kingSquare, occupied, Board::northWest);
   uint64_t kingSouthEast = Board::getNegativeRayAttacks(kingSquare, occupied, Board::southEast);
   
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

   masks.verMovable = ~(allInBetween ^ verInBetween);
   masks.diagMovable = ~(allInBetween ^ diagInBetween);
   masks.antiMovable = ~(allInBetween ^ antiInBetween);
   masks.pinned = allInBetween;

   uint64_t kingSuperOrth = kingNorth | kingSouth | kingEast | kingWest;
   uint64_t kingSuperDiag = kingNorthEast | kingNorthWest | kingSouthEast | kingSouthWest;

   uint64_t oppPawnAttacks;
   if constexpr (color == Board::white) 
      oppPawnAttacks = Board::blackPawnTargets(oppPawns);
   else 
      oppPawnAttacks = Board::whitePawnTargets(oppPawns);
   uint64_t pawnCheckFrom = Tables::pawnAttacks(color, kingSquare) & oppPawns;

   uint64_t oppAnyAttacks = oppPawnAttacks | oppSliderEast | oppSliderWest | oppSliderNorth | oppSliderSouth | oppSliderNorthEast | oppSliderSouthWest | oppSliderNorthWest 
         | oppSliderSouthEast | Board::knightAttackTargets(oppKnights) | Board::kingAttackTargets(oppKing);

   uint64_t blocks = allInBetween & empty;
   uint64_t checkFrom = (kingSuperOrth & oppRookLike) | (kingSuperDiag & oppBishopLike) 
         | pawnCheckFrom | (Tables::knightMoves(kingSquare) & oppKnights);

   int64_t nullIfCheck = Board::nullBoolMask(oppAnyAttacks & king);
   int64_t nullIfDoubleCheck = Board::nullBoolMask(checkFrom & (checkFrom - 1));
   uint64_t checkTo = checkFrom | blocks | nullIfCheck;

   masks.targetMask = ~pieces & checkTo & nullIfDoubleCheck;
   masks.pawnTargetMask = (masks.targetMask | (board.getEnPassantTarget(color) & Board::shift<up>(pawnCheckFrom))); //include en passant capture of pawn checker
   masks.kingTargetMask = ~(pieces | oppAnyAttacks);
   masks.notInCheck = nullIfCheck;
}

template<Board::PieceColor color>
bool generate(Board& board, MoveList& moveList, uint16_t ply) {
   Masks masks;
   std::array<uint16_t, NUM_SQUARES> indBuf;
   std::array<uint64_t, NUM_SQUARES> pinMasks;
   generateMasks<color>(board, masks, pinMasks, indBuf);

   appendPawnMoves<color>(board, moveList, indBuf, masks, ply);
   appendKnightMoves<color>(board, moveList, indBuf, masks, ply);
   appendSliderMoves<color, MoveType::bishop>(board, moveList, indBuf, pinMasks, masks, ply);
   appendSliderMoves<color, MoveType::rook>(board, moveList, indBuf, pinMasks, masks, ply);
   appendSliderMoves<color, MoveType::queen>(board, moveList, indBuf, pinMasks, masks, ply);
   appendKingMoves<color>(board, moveList, masks, ply);

   return !masks.notInCheck;
}

bool MoveGen::getLegalMoves(Board& board, MoveList& moveList, uint16_t ply) {
   assert(Tables::initialized);
   return board.getTurn() == Board::white ? generate<Board::white>(board, moveList, ply) : generate<Board::black>(board, moveList, ply); 
}