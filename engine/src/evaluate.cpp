#include "evaluate.hpp"

int16_t getSideMultiple(Board::PieceColor turn) { //return -1 for black and 1 for white
    return -turn + Board::getOppositeColor(turn);
}

int16_t pawnShelterBonus(GameState* state, Board::PieceColor turn) {
    constexpr uint64_t CASTLE_ZONES = 0xE7E700000000E7E7ULL;
    constexpr int16_t MAX_BONUS = 0;
    constexpr int16_t MISSING_PAWN_PENALTY = 10;
    constexpr int16_t OPEN_FILE_PENALTY = 12;

    int16_t bonus = MAX_BONUS;
    Board* board = state->getBoard();

    uint64_t pawns = board->getPieceSet(Board::pawns, turn), oppPawns = board->getPieceSet(Board::pawns, Board::getOppositeColor(turn));
    uint64_t king = board->getPieceSet(Board::king, turn);
    
    if(!(king & CASTLE_ZONES))
        return 0;
    
    uint64_t kingFile = Board::getFileMask(king), leftFile = Board::shift<Board::west>(kingFile);
    uint64_t currFile = leftFile ? leftFile : kingFile;
    for(int file = 0; file < 3; ++file) {
        if(!currFile)
            continue;

        uint64_t shelterPawn = currFile & pawns;
        if(shelterPawn) {
            currFile = Board::shift<Board::east>(currFile);
            continue;
        }

        int16_t penalty = MISSING_PAWN_PENALTY;
        if(!(currFile & oppPawns))
            penalty += OPEN_FILE_PENALTY;

        bonus -= penalty;
        currFile = Board::shift<Board::east>(currFile);
    }

    return bonus;
}

int16_t kingSafetyScore(GameState* state) {
    uint64_t pawnHash = state->getPawnHash();
    uint16_t ply = state->getPly();
    
    PawnTableEntry* pEntry = Tables::PTable.probe(pawnHash, ply);
    if(pEntry) 
        return pEntry->kingSafetyBonus;

    int16_t shelterBonus = pawnShelterBonus(state, Board::white) - pawnShelterBonus(state, Board::black);
    Tables::PTable.insert(pawnHash, ply, 0, shelterBonus);

    int16_t tropismScore = state->getTropism(Board::white) - state->getTropism(Board::black);
    return tropismScore + shelterBonus;
}

int16_t Eval::evaluate(GameState* state) {
    int16_t sideMultiple = getSideMultiple(state->getTurn());
    int16_t materialBalance = state->getMaterial(Board::white) - state->getMaterial(Board::black);

    int16_t pstScoreMid = state->getPst(Board::middle, Board::white) - state->getPst(Board::middle, Board::black);
    int16_t pstScoreEnd = state->getPst(Board::end, Board::white) - state->getPst(Board::end, Board::black);
 
    int16_t evalMid = materialBalance + pstScoreMid + kingSafetyScore(state);
    int16_t evalEnd = materialBalance + pstScoreEnd;
    
    int16_t phase = state->getPhase();
    int16_t eval = ((evalMid * (256 - phase)) + (evalEnd * phase)) / 256;
    
    return eval * sideMultiple;
}