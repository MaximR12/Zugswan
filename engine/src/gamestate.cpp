#include "gamestate.hpp"
#include <unordered_map>
#include <limits>
#include <chrono>

GameState::GameState() { 
    loadStartPos();
}

void GameState::updateCastleRights(Board::PieceColor fromColor, Board::PieceColor oppColor, uint64_t fromBB, uint64_t toBB) {
    uint64_t king = m_board.getPieceSet(Board::king, fromColor), rooks = m_board.getPieceSet(Board::rooks, fromColor);
    bool nullIfKingRookMove = !(fromBB & (king << 3));
    bool kingRookExists = rooks & (king << 3);
    bool nullIfQueenRookMove = !(fromBB & (king >> 4));
    bool queenRookExists = rooks & (king >> 4);
    bool nullIfKingMove = !(fromBB & king);

    uint8_t prevCastle = m_board.getCastleRights();
    bool prevKing = m_board.getKingCastleRights(fromColor), prevQueen = m_board.getQueenCastleRights(fromColor);
    m_board.updateKingCastleRights(fromColor, (nullIfKingRookMove & nullIfKingMove) & kingRookExists & prevKing);
    m_board.updateQueenCastleRights(fromColor, (nullIfQueenRookMove & nullIfKingMove) & queenRookExists & prevQueen);

    m_zobrist ^= Tables::ZTable.castleRights[prevCastle];
    m_zobrist ^= Tables::ZTable.castleRights[m_board.getCastleRights()];
}

void updateOccupied(Board& board, uint64_t occupied) {
    board.updateOccupiedBB(occupied);
    board.updateEmptyBB(~occupied);
}

void GameState::movePiece(Board::PieceType type, Board::PieceColor color, uint64_t fromToBB, uint16_t from, uint16_t to) {
    m_board.updateBB(Board::all, color, m_board.getPieceSet(Board::all, color) ^ fromToBB);
    m_board.updateBB(type, color, m_board.getPieceSet(type, color) ^ fromToBB);
    m_zobrist ^= Tables::ZTable.pieces[type+(color*PIECE_TYPES)][from];
    m_zobrist ^= Tables::ZTable.pieces[type+(color*PIECE_TYPES)][to];
    m_pstScores[Board::middle][color] -= Tables::pstScore(Board::middle, color, type, from);
    m_pstScores[Board::middle][color] += Tables::pstScore(Board::middle, color, type, to);
    m_pstScores[Board::end][color] -= Tables::pstScore(Board::end, color, type, from);
    m_pstScores[Board::end][color] += Tables::pstScore(Board::end, color, type, to);
}

void GameState::removePiece(Board::PieceType type, Board::PieceColor color, uint64_t pieceBB, uint16_t square) {
    m_board.updateBB(Board::all, color, m_board.getPieceSet(Board::all, color) ^ pieceBB);
    m_board.updateBB(type, color, m_board.getPieceSet(type, color) ^ pieceBB);
    m_zobrist ^= Tables::ZTable.pieces[type+(color*PIECE_TYPES)][square];
    m_pstScores[Board::middle][color] -= Tables::pstScore(Board::middle, color, type, square);
    m_pstScores[Board::end][color] -= Tables::pstScore(Board::end, color, type, square);
    m_phase += Board::getPiecePhase(type);
}

void GameState::addPiece(Board::PieceType type, Board::PieceColor color, uint64_t pieceBB, uint16_t square) {
    m_board.updateBB(Board::all, color, m_board.getPieceSet(Board::all, color) | pieceBB);
    m_board.updateBB(type, color, m_board.getPieceSet(type, color) | pieceBB);
    m_zobrist ^= Tables::ZTable.pieces[type+(color*PIECE_TYPES)][square];
    m_pstScores[Board::middle][color] += Tables::pstScore(Board::middle, color, type, square);
    m_pstScores[Board::end][color] += Tables::pstScore(Board::end, color, type, square);
    m_phase -= Board::getPiecePhase(type);
}

void GameState::makeMove(Move move) {
    Board::PieceColor turn = m_board.getTurn();
    
    m_undoStack.push_back(StateInfo{
        .epTarget=m_board.getEnPassantTarget(turn),
        .oppEpTarget=m_board.getEnPassantTarget(Board::getOppositeColor(turn)),
        .captureType=Board::invalid,
        .halfMoveClock=m_board.getHalfMoveClock(),
        .lastReversible=m_lastReversible,
        .kingCastleRights=m_board.getKingCastleRights(turn),
        .queenCastleRights=m_board.getQueenCastleRights(turn),
    });

    uint16_t fromInd = move.getFrom(), toInd = move.getTo();
    uint64_t fromBB = 1ULL << fromInd;
    uint64_t toBB = 1ULL << toInd; 
    uint64_t fromToBB = fromBB ^ toBB;

    Board::PieceColor oppColor = Board::getOppositeColor(turn);
    Board::PieceType fromType = m_board.getPieceType(fromInd, turn);

    updateCastleRights(turn, oppColor, fromBB, toBB);

    uint16_t flag = move.getFlag();
    if(!move.isCapture() || flag == EP_CAPTURE) {
        movePiece(fromType, turn, fromToBB, fromInd, toInd);
        updateOccupied(m_board, m_board.getOccupied() ^ fromToBB);
    } else {
        setLastReversible();
        m_board.resetHalfMoveClock();
        Board::PieceType captureType = m_board.getPieceType(toInd, oppColor);
        m_undoStack.back().captureType = captureType;
        movePiece(fromType, turn, fromToBB, fromInd, toInd);
        removePiece(captureType, oppColor, toBB, toInd);
        updateOccupied(m_board, m_board.getOccupied() ^ fromBB);
    }

    if(fromType == Board::pawns) {
        setLastReversible();
        m_board.resetHalfMoveClock();
        if(flag == DOUBLE_PAWN_PUSH) {
            uint64_t epTarget, epCapturers;
            if(turn == Board::white) {
                epTarget = Board::shift<Board::south>(toBB);
                epCapturers = m_board.whitePawnTargets(epTarget) & m_board.getPieceSet(Board::pawns, Board::black);
            } else {
                epTarget = Board::shift<Board::north>(toBB);
                epCapturers = m_board.blackPawnTargets(epTarget) & m_board.getPieceSet(Board::pawns, Board::white);
            }

            uint64_t empty = m_board.getEmpty();
            uint64_t emptyBeforeMove = empty | toBB; //hor pin test needs to happen before double pawn push move

            uint64_t rookLike = m_board.getPieceSet(Board::rooks, turn) | m_board.getPieceSet(Board::queens, turn);
            uint64_t oppKing = m_board.getPieceSet(Board::king, oppColor);

            uint64_t horInBetween = (Board::eastFill(rookLike, emptyBeforeMove) & Board::westFill(oppKing, emptyBeforeMove))
                | (Board::westFill(rookLike, emptyBeforeMove) & Board::eastFill(oppKing, emptyBeforeMove));
            uint64_t horPinnedMask = Board::nullBoolMask(horInBetween & epCapturers);
            epTarget &= horPinnedMask;

            m_board.updateEnPassantTargets(oppColor, epTarget);
            if(epTarget)
                m_zobrist ^= Tables::ZTable.epFiles[Board::getFile(epTarget)]; 
        } else if(flag == EP_CAPTURE) {
            uint64_t captureSquare;
            if(turn == Board::white)
                captureSquare = Board::shift<Board::south>(toBB);
            else
                captureSquare = Board::shift<Board::north>(toBB);
            Board::PieceType captureType = m_board.getPieceType(Board::serializeSingleBit(captureSquare), oppColor);
            m_undoStack.back().captureType = captureType;

            removePiece(captureType, oppColor, captureSquare, Board::serializeSingleBit(captureSquare));
            updateOccupied(m_board, m_board.getOccupied() ^ captureSquare);
        } else if(Move::isPromotion(flag)) {
            Board::PieceType promoType = Board::getPromoType(flag);
            m_board.updateBB(Board::pawns, turn, m_board.getPieceSet(Board::pawns, turn) & NOT_LAST_RANK);
            uint64_t promoSet = m_board.getPieceSet(promoType, turn);
            m_board.updateBB(promoType, turn, promoSet | toBB);

            m_zobrist ^= Tables::ZTable.pieces[Board::pawns+(turn*PIECE_TYPES)][toInd];
            m_zobrist ^= Tables::ZTable.pieces[promoType+(turn*PIECE_TYPES)][toInd];

            m_pstScores[Board::middle][turn] -= Tables::pstScore(Board::middle, turn, Board::pawns, toInd);
            m_pstScores[Board::middle][turn] += Tables::pstScore(Board::middle, turn, promoType, toInd);
            m_pstScores[Board::end][turn] -= Tables::pstScore(Board::end, turn, Board::pawns, toInd);
            m_pstScores[Board::end][turn] += Tables::pstScore(Board::end, turn, promoType, toInd);

            m_phase -= Board::getPiecePhase(promoType);
        }
    } else if(flag == KING_CASTLE) {
        setLastReversible();
        uint64_t rooks = m_board.getPieceSet(Board::rooks, turn);
        uint64_t kingRook = fromBB << 3;
        uint64_t rookFromToBB = kingRook | (kingRook >> 2);
        uint16_t from = Board::serializeSingleBit(kingRook), to = Board::serializeSingleBit(kingRook >> 2);

        movePiece(Board::rooks, turn, rookFromToBB, from, to);
        updateOccupied(m_board, m_board.getOccupied() ^ rookFromToBB);
    } else if(flag == QUEEN_CASTLE) {
        setLastReversible();
        uint64_t rooks = m_board.getPieceSet(Board::rooks, turn);
        uint64_t queenRook = fromBB >> 4;
        uint64_t rookFromToBB = queenRook | (queenRook << 3);
        uint16_t from = Board::serializeSingleBit(queenRook), to = Board::serializeSingleBit(queenRook << 3);

        movePiece(Board::rooks, turn, rookFromToBB, from, to);
        updateOccupied(m_board, m_board.getOccupied() ^ rookFromToBB); 
    }

    uint64_t epTarget = m_board.getEnPassantTarget(turn);
    if(epTarget){
        m_board.updateEnPassantTargets(turn, 0ULL);
        m_zobrist ^= Tables::ZTable.epFiles[Board::getFile(epTarget)]; 
    }

    switchTurn();
    m_board.incrementPly();
    m_hashList.push_back(m_zobrist);
    ++m_ply;

    assert(m_zobrist == GameState::calculateZobrist(&m_board));
}

void GameState::unmakeMove(Move move) {
    assert(m_undoStack.size() > 0);

    StateInfo& stateInfo = m_undoStack.back();
    uint16_t fromInd = move.getFrom(), toInd = move.getTo();
    uint64_t fromBB = 1ULL << fromInd;
    uint64_t toBB = 1ULL << toInd; 
    uint64_t fromToBB = fromBB ^ toBB;

    Board::PieceColor fromColor = Board::getOppositeColor(m_board.getTurn());
    Board::PieceColor oppColor = m_board.getTurn();
    Board::PieceType fromType = m_board.getPieceType(toInd, fromColor);

    uint16_t flag = move.getFlag();
    if(!move.isPromotion()) {
        if(!move.isCapture() || flag == EP_CAPTURE) {
            movePiece(fromType, fromColor, fromToBB, toInd, fromInd);
            updateOccupied(m_board, m_board.getOccupied() ^ fromToBB);
        } else {
            Board::PieceType captureType = stateInfo.captureType;
            movePiece(fromType, fromColor, fromToBB, toInd, fromInd);
            addPiece(captureType, oppColor, toBB, toInd);
            updateOccupied(m_board, m_board.getOccupied() | fromBB);
        }
    }

    if(flag == EP_CAPTURE) {
        uint64_t captureSquare;
        if(fromColor == Board::white)
            captureSquare = Board::shift<Board::south>(toBB);
        else
            captureSquare = Board::shift<Board::north>(toBB);
        Board::PieceType captureType = stateInfo.captureType;

        addPiece(captureType, oppColor, captureSquare, Board::serializeSingleBit(captureSquare));
        updateOccupied(m_board, m_board.getOccupied() | captureSquare);
    } else if(move.isPromotion()) {
        Board::PieceType promoType = Board::getPromoType(flag);
        removePiece(promoType, fromColor, toBB, toInd);
        addPiece(Board::pawns, fromColor, fromBB, fromInd);

        if(move.isCapture()) {
            addPiece(stateInfo.captureType, oppColor, toBB, toInd);
            updateOccupied(m_board, m_board.getOccupied() | fromBB);
        } else {
            updateOccupied(m_board, m_board.getOccupied() ^ fromToBB);
        }    
    } else if(flag == KING_CASTLE) {
        uint64_t rooks = m_board.getPieceSet(Board::rooks, fromColor);
        uint64_t kingRook = toBB >> 1;
        uint64_t rookFromToBB = kingRook | (fromBB << 3);
        uint16_t from = Board::serializeSingleBit(kingRook), to = Board::serializeSingleBit(fromBB << 3);

        movePiece(Board::rooks, fromColor, rookFromToBB, from, to);
        updateOccupied(m_board, m_board.getOccupied() ^ rookFromToBB);
    } else if(flag == QUEEN_CASTLE) {
        uint64_t rooks = m_board.getPieceSet(Board::rooks, fromColor);
        uint64_t queenRook = toBB << 1;
        uint64_t rookFromToBB = queenRook | (fromBB >> 4);
        uint16_t from = Board::serializeSingleBit(queenRook), to = Board::serializeSingleBit(fromBB >> 4);

        movePiece(Board::rooks, fromColor, rookFromToBB, from, to);
        updateOccupied(m_board, m_board.getOccupied() ^ rookFromToBB); 
    }

    if(stateInfo.epTarget) 
        m_zobrist ^= Tables::ZTable.epFiles[Board::getFile(stateInfo.epTarget)];
    uint64_t epTarget = m_board.getEnPassantTarget(oppColor);
    if(epTarget)
        m_zobrist ^= Tables::ZTable.epFiles[Board::getFile(epTarget)];
    m_board.updateEnPassantTargets(fromColor, stateInfo.epTarget);
    m_board.updateEnPassantTargets(oppColor, stateInfo.oppEpTarget);
    
    uint8_t prevCastle = m_board.getCastleRights();
    m_board.updateKingCastleRights(fromColor, stateInfo.kingCastleRights);
    m_board.updateQueenCastleRights(fromColor, stateInfo.queenCastleRights);
    m_zobrist ^= Tables::ZTable.castleRights[prevCastle];
    m_zobrist ^= Tables::ZTable.castleRights[m_board.getCastleRights()];

    m_board.updateHalfMoveClock(stateInfo.halfMoveClock);
    m_lastReversible = stateInfo.lastReversible;
    
    switchTurn();
    m_undoStack.pop_back();
    m_hashList.pop_back();
    --m_ply;

    assert(m_zobrist == GameState::calculateZobrist(&m_board));
}

void GameState::makeNullMove() {
    Board::PieceColor turn = m_board.getTurn();

    m_undoStack.push_back(StateInfo{
        .epTarget=m_board.getEnPassantTarget(turn),
    });

    uint64_t epTarget = m_board.getEnPassantTarget(turn);
    if(epTarget){
        m_board.updateEnPassantTargets(turn, 0ULL);
        m_zobrist ^= Tables::ZTable.epFiles[Board::getFile(epTarget)]; 
    }

    switchTurn();
    m_hashList.push_back(m_zobrist);
    m_board.incrementPly();
    ++m_ply;
}

void GameState::unmakeNullMove() {
    StateInfo& stateInfo = m_undoStack.back();

    Board::PieceColor fromColor = Board::getOppositeColor(m_board.getTurn());
    Board::PieceColor oppColor = m_board.getTurn();

    if(stateInfo.epTarget) 
        m_zobrist ^= Tables::ZTable.epFiles[Board::getFile(stateInfo.epTarget)];
    m_board.updateEnPassantTargets(fromColor, stateInfo.epTarget);

    switchTurn();
    m_board.decrementPly();
    m_undoStack.pop_back();
    m_hashList.pop_back();
    --m_ply;
}

bool GameState::isRepetition() const {
    for(int i = m_ply-2; i >= m_lastReversible+4; i-=2) { 
        if(m_hashList.compare(i, m_zobrist))
            return true;
    }
    return false;
}

bool GameState::shouldReduce(Move move) const {
    return !(m_inCheck || (m_board.getPieceType(move.getFrom(), m_board.getTurn()) == Board::pawns) 
        || Tables::isKiller(move, m_ply) || move.isGoodCapture() || move.isPromotion());
}

void GameState::setPst() {
    m_pstScores[Board::middle][Board::white] = GameState::calculatePstScore(&m_board, Board::middle, Board::white);
    m_pstScores[Board::middle][Board::black] = GameState::calculatePstScore(&m_board, Board::middle, Board::black);
    m_pstScores[Board::end][Board::white] = GameState::calculatePstScore(&m_board, Board::end, Board::white);
    m_pstScores[Board::end][Board::black] = GameState::calculatePstScore(&m_board, Board::end, Board::black);
}

void GameState::loadPosition(std::string fen) {
    m_board.loadPosition(fen);
    m_zobrist = GameState::calculateZobrist(&m_board);
    m_phase = GameState::calculatePhase(&m_board);
    m_lastReversible = 0;
    m_ply = 0;
    m_undoStack.clear();
    m_hashList.clear();
    m_hashList.push_back(m_zobrist);
    updateLegalMoves();
    setPst();
}

void GameState::moveFromList(std::vector<std::string>& moveList) {
    std::unordered_map<char, Board::PieceType> charPieceMap {
        {'n', Board::knights}, {'b', Board::bishops}, {'r', Board::rooks}, {'q', Board::queens} 
    };

    for(int i = 0; i < moveList.size(); ++i) {
        std::string moveStr = moveList[i];
        uint16_t from = Board::getIndexSquare(moveStr.substr(0, 2));
        uint16_t to = Board::getIndexSquare(moveStr.substr(2, 2));
        Board::PieceType promoType = moveStr.size() == 4 ? Board::invalid : charPieceMap[moveStr[4]];

        for(int j = 0; j < m_legalMoves.size(); ++j) {
            Move curr = m_legalMoves[j];
            if(curr.isPromotion() && (Board::getPromoType(curr.getFlag()) != promoType))
                continue;

            if(curr.getFrom() == from && curr.getTo() == to) {                
                makeMove(curr);
                updateLegalMoves();
                break;
            }    
        } 
    }
}

int16_t GameState::calculatePhase(Board* board) {
    int16_t phase = totalPhase;
    for(int type = Board::pawns; type < Board::king; ++type) {
        Board::PieceType pieceType = static_cast<Board::PieceType>(type);
        int16_t totalCount = (board->getPieceCount(pieceType, Board::white) + board->getPieceCount(pieceType, Board::black));
        phase -= Board::getPiecePhase(type) * (board->getPieceCount(pieceType, Board::white) + board->getPieceCount(pieceType, Board::black));
    }

    return phase;
}

int16_t GameState::calculatePstScore(Board* board, Board::Phase phase, Board::PieceColor turn) {
    int16_t pst = 0;
    for(int pt = Board::pawns; pt <= Board::king; ++pt) {
        Board::PieceType pieceType = static_cast<Board::PieceType>(pt);
        std::array<uint16_t, NUM_SQUARES> indBuf;
        size_t numPieces = Board::serializeBitboard(board->getPieceSet(pieceType, turn), indBuf);
        for(int i = 0; i < numPieces; ++i) 
            pst += Tables::pstScore(phase, turn, pieceType, indBuf[i]);
    }

    return pst;
}

uint64_t GameState::calculateZobrist(Board* board) {
    uint64_t zobrist = 0;
    std::array<uint16_t, NUM_SQUARES> indBuf;
    for(int color = Board::white; color <= Board::black; ++color) {
        for(int type = Board::pawns; type <= Board::king; ++type) {
            uint64_t currSet = board->getPieceSet(static_cast<Board::PieceType>(type), static_cast<Board::PieceColor>(color)); 
            size_t size = Board::serializeBitboard(currSet, indBuf);

            for(int i = 0; i < size; ++i) 
                zobrist ^= Tables::ZTable.pieces[type+(PIECE_TYPES*color)][indBuf[i]];
        }
    }

    Board::PieceColor turn = board->getTurn();
    uint64_t epTarget = board->getEnPassantTarget(turn);
    if(epTarget) 
        zobrist ^= Tables::ZTable.epFiles[Board::getFile(epTarget)];

    if(turn == Board::black)
        zobrist ^= Tables::ZTable.blackSide;

    zobrist ^= Tables::ZTable.castleRights[board->getCastleRights()];

    return zobrist;
}