#include "gamestate.hpp"
#include <chrono>

GameState::GameState() : m_state{State::inProgress}, m_turn{Board::white}, m_legalMoves{} { 
    loadStartPos();
}

void updateCastleRights(Board& board, Board::PieceColor fromColor, uint64_t fromBB) {
    uint64_t king = board.getPieceSet(Board::king, fromColor);
    uint64_t nullIfKingRookMove = Board::nullBoolMask(fromBB & king << 3);
    uint64_t nullIfQueenRookMove = Board::nullBoolMask(fromBB & king >> 4);
    uint64_t nullIfKingMove = Board::nullBoolMask(fromBB & king);

    board.updateKingCastleRights(fromColor, (nullIfKingRookMove & nullIfKingMove) & Board::fullBoolMask(board.getKingCastleRights(fromColor)));
    board.updateQueenCastleRights(fromColor, (nullIfQueenRookMove & nullIfKingMove) & Board::fullBoolMask(board.getQueenCastleRights(fromColor)));
}

void updateOccupied(Board& board, uint64_t occupied) {
    board.updateOccupiedBB(occupied);
    board.updateEmptyBB(~occupied);
}

void movePiece(Board& board, Board::PieceType type, Board::PieceColor color, uint64_t fromToBB) {
    board.updateBB(Board::all, color, board.getPieceSet(Board::all, color) ^ fromToBB);
    board.updateBB(type, color, board.getPieceSet(type, color) ^ fromToBB);
}

void removePiece(Board& board, Board::PieceType type, Board::PieceColor color, uint64_t pieceBB) {
    board.updateBB(Board::all, color, board.getPieceSet(Board::all, color) ^ pieceBB);
    board.updateBB(type, color, board.getPieceSet(type, color) ^ pieceBB);
}

void addPiece(Board& board, Board::PieceType type, Board::PieceColor color, uint64_t pieceBB) {
    board.updateBB(Board::all, color, board.getPieceSet(Board::all, color) | pieceBB);
    board.updateBB(type, color, board.getPieceSet(type, color) | pieceBB);
}

void GameState::makeMove(Move move) {
    m_undoStack.push_back(StateInfo{
        m_board.getEnPassantTarget(m_turn), Board::invalid, m_board.getHalfMoveClock(), m_board.getKingCastleRights(m_turn), m_board.getQueenCastleRights(m_turn)
    });

    uint16_t fromInd = move.getFrom(), toInd = move.getTo();
    uint64_t fromBB = 1ULL << fromInd;
    uint64_t toBB = 1ULL << toInd; 
    uint64_t fromToBB = fromBB ^ toBB;

    Board::PieceColor fromColor = m_turn;
    Board::PieceColor oppColor = Board::getOppositeColor(fromColor);
    Board::PieceType fromType = m_board.getPieceType(fromInd);

    updateCastleRights(m_board, fromColor, fromBB);
    m_board.updateEnPassantTargets(oppColor, 0ULL);

    uint16_t flag = move.getFlag();
    if(!move.isCapture() || flag == EP_CAPTURE) {
        movePiece(m_board, fromType, fromColor, fromToBB);
        updateOccupied(m_board, m_board.getOccupied() ^ fromToBB);
    } else {
        Board::PieceType captureType = m_board.getPieceType(toInd);
        m_undoStack.back().captureType = captureType;
        movePiece(m_board, fromType, fromColor, fromToBB);
        removePiece(m_board, captureType, oppColor, toBB);
        updateOccupied(m_board, m_board.getOccupied() ^ fromBB);
    }

    if(fromType == Board::pawns) {
        if(flag == DOUBLE_PAWN_PUSH) {
            uint64_t epTarget, epCapturers;
            if(fromColor == Board::white) {
                epTarget = Board::shift<Board::south>(toBB);
                epCapturers = m_board.whitePawnTargets(epTarget) & m_board.getPieceSet(Board::pawns, Board::black);
            } else {
                epTarget = Board::shift<Board::north>(toBB);
                epCapturers = m_board.blackPawnTargets(epTarget) & m_board.getPieceSet(Board::pawns, Board::white);
            }

            uint64_t empty = m_board.getEmpty();
            uint64_t emptyBeforeMove = empty | toBB; //hor pin test needs to happen before double pawn push move

            uint64_t rookLike = m_board.getPieceSet(Board::rooks, fromColor) | m_board.getPieceSet(Board::queens, fromColor);
            uint64_t oppKing = m_board.getPieceSet(Board::king, oppColor);

            uint64_t horInBetween = (Board::eastFill(rookLike, emptyBeforeMove) & Board::westFill(oppKing, emptyBeforeMove))
                | (Board::westFill(rookLike, emptyBeforeMove) & Board::eastFill(oppKing, emptyBeforeMove));
            uint64_t horPinnedMask = Board::nullBoolMask(horInBetween & epCapturers);

            m_board.updateEnPassantTargets(oppColor, epTarget & horPinnedMask);
        } else if(flag == EP_CAPTURE) {
            uint64_t captureSquare;
            if(fromColor == Board::white)
                captureSquare = Board::shift<Board::south>(toBB);
            else
                captureSquare = Board::shift<Board::north>(toBB);
            Board::PieceType captureType = m_board.getPieceType(Board::serializeSingleBit(captureSquare));
            m_undoStack.back().captureType = captureType;

            removePiece(m_board, captureType, oppColor, captureSquare);
            updateOccupied(m_board, m_board.getOccupied() ^ captureSquare);
        } else if(Move::isPromotion(flag)) {
            Board::PieceType promoType = Board::getPromoType(flag);
            m_board.updateBB(Board::pawns, fromColor, m_board.getPieceSet(Board::pawns, fromColor) & NOT_LAST_RANK);
            uint64_t promoSet = m_board.getPieceSet(promoType, fromColor);
            m_board.updateBB(promoType, fromColor, promoSet | toBB);
        }
    } else if(flag == KING_CASTLE) {
        uint64_t rooks = m_board.getPieceSet(Board::rooks, fromColor);
        uint64_t kingRook = fromBB << 3;
        uint64_t rookFromToBB = kingRook | (kingRook >> 2);

        movePiece(m_board, Board::rooks, fromColor, rookFromToBB);
        updateOccupied(m_board, m_board.getOccupied() ^ rookFromToBB);
    } else if(flag == QUEEN_CASTLE) {
        uint64_t rooks = m_board.getPieceSet(Board::rooks, fromColor);
        uint64_t queenRook = fromBB >> 4;
        uint64_t rookFromToBB = queenRook | (queenRook << 3);

        movePiece(m_board, Board::rooks, fromColor, rookFromToBB);
        updateOccupied(m_board, m_board.getOccupied() ^ rookFromToBB); 
    }

    switchTurn();
}

void GameState::unMakeMove(Move move) {
    assert(m_undoStack.size() > 0);

    StateInfo& stateInfo = m_undoStack.back();
    uint16_t fromInd = move.getFrom(), toInd = move.getTo();
    uint64_t fromBB = 1ULL << fromInd;
    uint64_t toBB = 1ULL << toInd; 
    uint64_t fromToBB = fromBB ^ toBB;

    Board::PieceColor fromColor = Board::getOppositeColor(m_turn);
    Board::PieceColor oppColor = m_turn;
    Board::PieceType fromType = m_board.getPieceType(toInd);

    uint16_t flag = move.getFlag();
    if(!move.isPromotion()) {
        if(!move.isCapture() || flag == EP_CAPTURE) {
            movePiece(m_board, fromType, fromColor, fromToBB);
            updateOccupied(m_board, m_board.getOccupied() ^ fromToBB);
        } else {
            Board::PieceType captureType = stateInfo.captureType;
            movePiece(m_board, fromType, fromColor, fromToBB);
            addPiece(m_board, captureType, oppColor, toBB);
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

        addPiece(m_board, captureType, oppColor, captureSquare);
        updateOccupied(m_board, m_board.getOccupied() | captureSquare);
    } else if(move.isPromotion()) {
        Board::PieceType promoType = Board::getPromoType(flag);
        removePiece(m_board, promoType, fromColor, toBB);
        addPiece(m_board, Board::pawns, fromColor, fromBB);

        if(move.isCapture()) {
            addPiece(m_board, stateInfo.captureType, oppColor, toBB);
            updateOccupied(m_board, m_board.getOccupied() | fromBB);
        } else {
            updateOccupied(m_board, m_board.getOccupied() ^ fromToBB);
        }    
    } else if(flag == KING_CASTLE) {
        uint64_t rooks = m_board.getPieceSet(Board::rooks, fromColor);
        uint64_t kingRook = toBB >> 1;
        uint64_t rookFromToBB = kingRook | (fromBB << 3);

        movePiece(m_board, Board::rooks, fromColor, rookFromToBB);
        updateOccupied(m_board, m_board.getOccupied() ^ rookFromToBB);
    } else if(flag == QUEEN_CASTLE) {
        uint64_t rooks = m_board.getPieceSet(Board::rooks, fromColor);
        uint64_t queenRook = toBB << 1;
        uint64_t rookFromToBB = queenRook | (fromBB >> 4);

        movePiece(m_board, Board::rooks, fromColor, rookFromToBB);
        updateOccupied(m_board, m_board.getOccupied() ^ rookFromToBB); 
    }

    m_board.updateEnPassantTargets(fromColor, stateInfo.epTarget);
    m_board.updateKingCastleRights(fromColor, stateInfo.kingCastleRights);
    m_board.updateQueenCastleRights(fromColor, stateInfo.queenCastleRights);
    m_board.updateHalfMoveClock(stateInfo.halfMoveClock);
    
    m_undoStack.pop_back();
    switchTurn();
}

void GameState::loadPosition(std::string fen) {
    m_turn = m_board.loadPosition(fen);
    m_undoStack.clear();
    updateLegalMoves();
}

void GameState::moveFromList(std::vector<std::string>& moveList) {
    for(int i = 0; i < moveList.size(); ++i) {
        std::string moveStr = moveList[i];
        uint16_t from = Board::getIndexSquare(moveStr.substr(0, 2));
        uint16_t to = Board::getIndexSquare(moveStr.substr(2, 2));

        for(int j = 0; j < m_legalMoves.size(); ++j) {
            Move curr = m_legalMoves[j];
            if(curr.getFrom() == from && curr.getTo() == to) {
                makeMove(curr);
                updateLegalMoves();
                break;
            }    
        } 
    }
}