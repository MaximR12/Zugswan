#include "evaluate.hpp"
#include "search.hpp"

struct Score {
    int16_t mgScore;
    int16_t egScore;

    Score operator-(const Score& other) {
        return Score(this->mgScore - other.mgScore, this->egScore - other.egScore);
    }
};

int16_t getSideMultiple(Board::PieceColor turn) { //return -1 for black and 1 for white
    return -turn + Board::getOppositeColor(turn);
}

bool isBackward(uint64_t pawnBB, uint64_t pawnSet, uint64_t oppPawnControl, Board::PieceColor turn, Board::Directions pawnDir) {
    uint64_t adjacentSquares = Board::shift<Board::east>(pawnBB) | Board::shift<Board::west>(pawnBB);
    while(adjacentSquares) {
        uint16_t currSquare = Board::bitScanForward(adjacentSquares);
        uint64_t possibleSupporters = Tables::getRayMoves(currSquare, Board::getOppositeDirection(pawnDir));
        if(possibleSupporters & pawnSet)
            return false;

        adjacentSquares &= adjacentSquares-1;
    }

    uint64_t stopSquareBB = Board::shift<Board::north>(pawnBB);
    bool stopSquareProtected = turn == Board::white ? Board::blackPawnTargets(stopSquareBB) & pawnSet : Board::whitePawnTargets(stopSquareBB) & pawnSet;
    bool stopSquareControlled = stopSquareBB & oppPawnControl;

    return !stopSquareProtected && stopSquareControlled;
}

Score calcPawnScore(Board* board, uint16_t pawnInd, Board::PieceColor turn) {
    constexpr Score DOUBLED_PENALTY = {10, 12};
    constexpr Score BACKWARD_PENALTY = {10, 16};
    constexpr Score ISOLATED_PENALTY = {12, 18};
    constexpr int16_t OPEN_FILE_PENALTY = 8; //to be added to backward or isolated pawns
    constexpr int16_t PROTECTED_PASSED_BONUS = 15;
    constexpr std::array<Score, 8> PASSED_BONUSES {{
        {0, 0}, {0, 0}, {5, 10}, {10, 20}, {20, 40}, {35, 70}, {60, 120}, {0, 0}
    }};

    Score score = {0, 0};
    Board::Directions pawnDir = turn == Board::white ? Board::north : Board::south;
    uint64_t pawnBB = 1ULL<<pawnInd;
    uint64_t pawnSet = board->getPieceSet(Board::pawns, turn);
    uint64_t pawnPath = Tables::getRayMoves(pawnInd, pawnDir);
    
    if(pawnPath & pawnSet) { //doubled
        score.mgScore -= DOUBLED_PENALTY.mgScore;
        score.egScore -= DOUBLED_PENALTY.egScore;
    }

    uint64_t oppPawnSet = board->getPieceSet(Board::pawns, Board::getOppositeColor(turn));
    uint64_t fileMask = Board::getFileMask(pawnBB);
    uint64_t adjacentPawns = (Board::shift<Board::west>(fileMask) & pawnSet) | (Board::shift<Board::east>(fileMask) & pawnSet);
    uint64_t oppPawnControl = turn == Board::white ? Board::blackPawnTargets(oppPawnSet) : Board::whitePawnTargets(oppPawnSet);
    bool isOpenFile = !(pawnPath & oppPawnSet);
    bool pawnEndgame = board->pawnEndgame();

    if(!pawnEndgame && !adjacentPawns) { //isolated
        score.mgScore -= ISOLATED_PENALTY.mgScore + (OPEN_FILE_PENALTY*isOpenFile);
        score.egScore -= ISOLATED_PENALTY.egScore + (OPEN_FILE_PENALTY*isOpenFile);
    } else if(!pawnEndgame && isBackward(pawnBB, pawnSet, oppPawnControl, turn, pawnDir)) { //backward
        score.mgScore -= BACKWARD_PENALTY.mgScore + (OPEN_FILE_PENALTY*isOpenFile);
        score.egScore -= BACKWARD_PENALTY.egScore + (OPEN_FILE_PENALTY*isOpenFile);
    }

    if(!(pawnPath & (oppPawnControl | oppPawnSet))) { //passed

        int rank = Board::getRank(pawnBB);
        score.mgScore += PASSED_BONUSES[turn == Board::white ? rank : 7-rank].mgScore;
        score.egScore += PASSED_BONUSES[turn == Board::white ? rank : 7-rank].egScore;

        uint64_t supportingSquares = turn == Board::white ? Board::blackPawnTargets(pawnBB) : Board::whitePawnTargets(pawnBB); 
        if(supportingSquares & pawnSet) {
            score.mgScore += PROTECTED_PASSED_BONUS;
            score.egScore += PROTECTED_PASSED_BONUS;
        }
    } 

    return score;
}

Score evalPawnStructure(GameState* state, Board::PieceColor turn) {
    Score score = {0, 0};
    
    Board* board = state->getBoard();
    std::array<uint16_t, NUM_SQUARES> indBuf;
    size_t size = Board::serializeBitboard(board->getPieceSet(Board::pawns, turn), indBuf);
    for(int i = 0; i < size; ++i) {
        Score pawnScore = calcPawnScore(board, indBuf[i], turn); 
        score.mgScore += pawnScore.mgScore;
        score.egScore += pawnScore.egScore; 
    }

    return score;
}

Score getPawnScore(GameState* state) {
    uint64_t pawnHash = state->getPawnHash();
    uint16_t ply = state->getPly();

    PawnTableEntry* pEntry = Tables::PTable.probe(pawnHash, ply);
    if(pEntry && pEntry->mgPawnScore != VALUE_UNDEFINED && pEntry->egPawnScore != VALUE_UNDEFINED)
        return {pEntry->mgPawnScore, pEntry->egPawnScore};

    Score pawnScore = evalPawnStructure(state, Board::white) - evalPawnStructure(state, Board::black);
    if(pEntry) {
        pEntry->mgPawnScore = pawnScore.mgScore;
        pEntry->egPawnScore = pawnScore.egScore;
    } else
        Tables::PTable.insert(pawnHash, ply, pawnScore.mgScore, pawnScore.egScore, VALUE_UNDEFINED);
        
    return pawnScore;
}

int16_t pawnShelterBonus(GameState* state, Board::PieceColor turn) {
    constexpr uint64_t CASTLE_ZONES = 0xE7E700000000E7E7ULL;
    constexpr int16_t MAX_BONUS = 10;
    constexpr int16_t MISSING_PAWN_PENALTY = 15;
    constexpr int16_t OPEN_FILE_PENALTY = 15;
    constexpr std::array<int16_t, 5> DISTANCE_PENALTIES = {
        0, 5, 10, 12, 15
    };

    int16_t bonus = MAX_BONUS;
    Board* board = state->getBoard();

    uint64_t pawns = board->getPieceSet(Board::pawns, turn), oppPawnSet = board->getPieceSet(Board::pawns, Board::getOppositeColor(turn));
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
            if(shelterPawn & shelterPawn-1) //if multiple shelter pawns, consider distance of closest to starting rank
                shelterPawn = turn == Board::white ? 1ULL<<Board::bitScanForward(shelterPawn) : 1ULL<<Board::bitScanReverse(shelterPawn);

            int secondRank = turn == Board::white ? 1 : 6, currPawnRank = Board::getRank(shelterPawn);
            int distance = std::min(std::abs(currPawnRank - secondRank), 4);

            int16_t distancePenalty = DISTANCE_PENALTIES[distance];
            bonus -= distancePenalty;

            currFile = Board::shift<Board::east>(currFile);
            continue;
        }

        int16_t penalty = MISSING_PAWN_PENALTY;
        if(!(currFile & oppPawnSet))
            penalty += OPEN_FILE_PENALTY;

        bonus -= penalty;
        currFile = Board::shift<Board::east>(currFile);
    }

    return bonus;
}

int16_t kingSafetyScore(GameState* state) {
    uint64_t pawnHash = state->getPawnHash();
    uint16_t ply = state->getPly();
    
    int16_t shelterBonus = pawnShelterBonus(state, Board::white) - pawnShelterBonus(state, Board::black);
    PawnTableEntry* pEntry = Tables::PTable.probe(pawnHash, ply);
    if(pEntry && pEntry->kingSafetyBonus != VALUE_UNDEFINED)
        shelterBonus = pEntry->kingSafetyBonus;
    else if(pEntry) {
        shelterBonus = pawnShelterBonus(state, Board::white) - pawnShelterBonus(state, Board::black);
        pEntry->kingSafetyBonus = shelterBonus;
    } else {
        shelterBonus = pawnShelterBonus(state, Board::white) - pawnShelterBonus(state, Board::black);
        Tables::PTable.insert(pawnHash, ply, VALUE_UNDEFINED, VALUE_UNDEFINED, shelterBonus);
    }

    int16_t tropismScore = state->getTropism(Board::white) - state->getTropism(Board::black);
    return tropismScore + shelterBonus;
}

int16_t Eval::evaluate(GameState* state) {
    constexpr int16_t TEMPO_BONUS = 10;
    int16_t sideMultiple = getSideMultiple(state->getTurn());
    int16_t materialBalance = state->getMaterial(Board::white) - state->getMaterial(Board::black);

    int16_t pstScoreMid = state->getPst(Board::middle, Board::white) - state->getPst(Board::middle, Board::black);
    int16_t pstScoreEnd = state->getPst(Board::end, Board::white) - state->getPst(Board::end, Board::black);

    Score pawnScore = getPawnScore(state);
 
    int16_t evalMid = materialBalance + pstScoreMid + kingSafetyScore(state) + pawnScore.mgScore;
    int16_t evalEnd = materialBalance + pstScoreEnd + pawnScore.egScore;
    
    int16_t phase = state->getPhase();
    int16_t eval = ((evalMid * (256 - phase)) + (evalEnd * phase)) / 256;
    
    return eval * sideMultiple + TEMPO_BONUS;
}