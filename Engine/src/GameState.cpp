#include "GameState.h"
#include <chrono>

GameState::GameState() : m_playerTurn{Board::white}, m_turn{Board::white}, m_moveGen{m_board, &m_tables}, m_engine{m_board, &m_moveGen}, m_currLegalMoves{}, 
        m_selectedSquare{UNDEFINED_SQUARE} 
{ 
    m_boardStack.reserve(INIT_STACK_SIZE);
    m_boardStack.emplace_back();
    updateBoard();

    m_numLegalMoves = m_moveGen.getLegalMoves(m_playerTurn, m_currLegalMoves);     
}

void updateCastleRights(Board* board, Board::PieceColor fromColor, uint64_t fromBB) {
    uint64_t king = board->getPieceSet(Board::king, fromColor);
    uint64_t nullIfKingRookMove = Board::nullBoolMask(fromBB & king << 3);
    uint64_t nullIfQueenRookMove = Board::nullBoolMask(fromBB & king >> 4);
    uint64_t nullIfKingMove = Board::nullBoolMask(fromBB & king);

    board->updateKingCastleRights(fromColor, (nullIfKingRookMove & nullIfKingMove) & Board::fullBoolMask(board->getKingCastleRights(fromColor)));
    board->updateQueenCastleRights(fromColor, (nullIfQueenRookMove & nullIfKingMove) & Board::fullBoolMask(board->getQueenCastleRights(fromColor)));
}

void updateOccupied(Board* board, uint64_t occupied) {
    board->updateOccupiedBB(occupied);
    board->updateEmptyBB(~occupied);
}

void movePiece(Board* board, Board::PieceType type, Board::PieceColor color, uint64_t fromToBB) {
    board->updateBB(Board::all, color, board->getPieceSet(Board::all, color) ^ fromToBB);
    board->updateBB(type, color, board->getPieceSet(type, color) ^ fromToBB);
}

void removePiece(Board* board, Board::PieceType type, Board::PieceColor color, uint64_t pieceBB) {
    board->updateBB(Board::all, color, board->getPieceSet(Board::all, color) ^ pieceBB);
    board->updateBB(type, color, board->getPieceSet(type, color) ^ pieceBB);
}

void GameState::makeMove(Move move) {
    m_boardStack.emplace_back(*m_board);
    updateBoard();

    uint16_t fromInd = move.getFrom(), toInd = move.getTo();
    uint64_t fromBB = 1ULL << fromInd;
    uint64_t toBB = 1ULL << toInd; 
    uint64_t fromToBB = fromBB ^ toBB;

    Board::PieceColor fromColor = m_board->getPieceColor(fromInd);
    Board::PieceColor oppColor = Board::getOppositeColor(fromColor);
    Board::PieceType fromType = m_board->getPieceType(fromInd);
    uint64_t fromColorBB = m_board->getPieceSet(Board::all, fromColor);
    uint64_t fromTypeBB = m_board->getPieceSet(fromType, fromColor);

    updateCastleRights(m_board, fromColor, fromBB);
    m_board->updateEnPassantTargets(oppColor, 0ULL);

    uint16_t flag = move.getFlag();
    if(!move.isCapture() || flag == EP_CAPTURE) {
        movePiece(m_board, fromType, fromColor, fromToBB);
        updateOccupied(m_board, m_board->getOccupied() ^ fromToBB);
    } else {
        Board::PieceType captureType = m_board->getPieceType(toInd);
        movePiece(m_board, fromType, fromColor, fromToBB);
        removePiece(m_board, captureType, oppColor, toBB);
        updateOccupied(m_board, m_board->getOccupied() ^ fromBB);
    }
    
    if(flag != EP_CAPTURE) flag &= ~CAPTURE; //consider promotions and promo captures in the same case
    switch(flag) {
        case DOUBLE_PAWN_PUSH:
        {
            //horizontal pin test of possible capturing pawns
            uint64_t epTarget = Board::pawnShift(toBB, Board::getPawnDirection(oppColor));
            uint64_t epCapturers = m_board->pawnAttackTargets(epTarget, fromColor) & m_board->getPieceSet(Board::pawns, oppColor);
            uint64_t empty = m_board->getEmpty();
            uint64_t emptyBeforeMove = empty | toBB; //hor pin test needs to happen before double pawn push move

            uint64_t rookLike = m_board->getPieceSet(Board::rooks, fromColor) | m_board->getPieceSet(Board::queens, fromColor);
            uint64_t oppKing = m_board->getPieceSet(Board::king, oppColor);

            uint64_t horInBetween = (Board::eastFill(rookLike, emptyBeforeMove) & Board::westFill(oppKing, emptyBeforeMove))
                | (Board::westFill(rookLike, emptyBeforeMove) & Board::eastFill(oppKing, emptyBeforeMove));
            uint64_t horPinnedMask = Board::nullBoolMask(horInBetween & epCapturers);

            m_board->updateEnPassantTargets(oppColor, epTarget & horPinnedMask);
            break;
        }

        case KING_CASTLE:
        {
            uint64_t rooks = m_board->getPieceSet(Board::rooks, fromColor);
            uint64_t kingRook = fromBB << 3;
            uint64_t rookFromToBB = kingRook | (kingRook >> 2);

            movePiece(m_board, Board::rooks, fromColor, rookFromToBB);
            updateOccupied(m_board, m_board->getOccupied() ^ rookFromToBB);
            break;
        }

        case QUEEN_CASTLE:
        {
            uint64_t rooks = m_board->getPieceSet(Board::rooks, fromColor);
            uint64_t queenRook = fromBB >> 4;
            uint64_t rookFromToBB = queenRook | (queenRook << 3);

            movePiece(m_board, Board::rooks, fromColor, rookFromToBB);
            updateOccupied(m_board, m_board->getOccupied() ^ rookFromToBB);   
            break;
        }

        case EP_CAPTURE:
        {
            uint64_t captureSquare = Board::pawnShift(toBB, Board::getPawnDirection(oppColor));
            Board::PieceType captureType = m_board->getPieceType(Board::serializeSingleBit(captureSquare));
            uint64_t captureTypeBB = m_board->getPieceSet(captureType, oppColor);

            removePiece(m_board, captureType, oppColor, captureSquare);
            updateOccupied(m_board, m_board->getOccupied() ^ captureSquare);
            break;
        }

        case KNIGHT_PROMOTION:
        {
            m_board->updateBB(Board::pawns, fromColor, m_board->getPieceSet(Board::pawns, fromColor) & NOT_LAST_RANK);
            uint64_t knightSet = m_board->getPieceSet(Board::knights, fromColor);
            m_board->updateBB(Board::knights, fromColor, knightSet | toBB);
            break;
        }
            
        case BISHOP_PROMOTION:
        {
            m_board->updateBB(Board::pawns, fromColor, m_board->getPieceSet(Board::pawns, fromColor) & NOT_LAST_RANK);
            uint64_t bishopSet = m_board->getPieceSet(Board::bishops, fromColor);
            m_board->updateBB(Board::bishops, fromColor, bishopSet | toBB);
            break;
        }
            
        case ROOK_PROMOTION:
        {
            m_board->updateBB(Board::pawns, fromColor, m_board->getPieceSet(Board::pawns, fromColor) & NOT_LAST_RANK);
            uint64_t rookSet = m_board->getPieceSet(Board::rooks, fromColor);
            m_board->updateBB(Board::rooks, fromColor, rookSet | toBB);
            break;
        }
            
        case QUEEN_PROMOTION:
        {
            m_board->updateBB(Board::pawns, fromColor, m_board->getPieceSet(Board::pawns, fromColor) & NOT_LAST_RANK);
            uint64_t queenSet = m_board->getPieceSet(Board::queens, fromColor);
            m_board->updateBB(Board::queens, fromColor, queenSet | toBB);
            break;
        }

        default:
            break;
    }    

    switchTurn();
}

void GameState::unMakeMove() {
    if(m_boardStack.size() > 1) {
        m_boardStack.pop_back();
        updateBoard();
        updateLegalMoves();
        switchTurn();
    }
}

void GameState::playEngineMove() {
    const Move topMove = m_engine.getTopMove(m_turn);
    makeMove(topMove);
}

void GameState::handleClick(int square) {
    for(const Move move : m_currLegalMoves) {
        if(m_selectedSquare == move.getFrom() && square == move.getTo()) {
            m_selectedSquare = UNDEFINED_SQUARE;
            makeMove(move);
            return;
        }
    }

    m_selectedSquare = square;
} 

void GameState::loadPosition(std::string FEN) {
    m_boardStack.clear();
    m_boardStack.emplace_back();
    m_turn = m_boardStack.back().loadPosition(FEN);
    updateBoard();

    m_numLegalMoves = m_moveGen.getLegalMoves(m_playerTurn, m_currLegalMoves);
}

void appendPieces(std::vector<Piece>& pieceList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint16_t numPieces, const Board::PieceColor& color, const Board::PieceType& type) {
    for(int i = 0; i < numPieces; ++i) {
        int ind = indBuf[i];
        int row = ind / ROW_LEN, col = ind % ROW_LEN;
        pieceList.emplace_back(color, type, row, col);
    }
}

std::vector<Piece> GameState::getPieceList() const {
    std::vector<Piece> pieceList;
    pieceList.reserve(NUM_SQUARES);
    std::array<uint16_t, NUM_SQUARES> indBuf;

    for(int pieceColor = Board::white; pieceColor <= Board::black; ++pieceColor) {
        for(int pieceType = Board::pawns; pieceType <= Board::king; ++pieceType) {
            Board::PieceType type = static_cast<Board::PieceType>(pieceType);
            Board::PieceColor color = static_cast<Board::PieceColor>(pieceColor);
            uint16_t numPieces = Board::serializeBitboard(m_board->getPieceSet(type, color), indBuf);

            appendPieces(pieceList, indBuf, numPieces, color, type);
        }
    }

    return pieceList;
}