#include "GameState.h"
#include <unordered_map>

void GameState::makeMove(Move move) {
    uint16_t fromInd = move.getFrom(), toInd = move.getTo();
    uint64_t fromBB = 1ULL << fromInd;
    uint64_t toBB = 1ULL << toInd; 
    uint64_t fromToBB = fromBB ^ toBB;

    Board::PieceColor fromColor = m_board.getPieceColor(fromInd);
    Board::PieceColor oppColor = Board::getOppositeColor(fromColor);
    Board::PieceType fromType = m_board.getPieceType(fromInd);
    uint64_t fromColorBB = m_board.getPieceSet(Board::all, fromColor);
    uint64_t fromTypeBB = m_board.getPieceSet(fromType, fromColor);

    uint16_t flag = move.getFlag();
    m_board.updateEnPassantTargets(oppColor, 0ULL);
    if(!move.isCapture() || flag == EP_CAPTURE) {
        m_board.updateBB(Board::all, fromColor, fromColorBB ^ fromToBB);
        m_board.updateBB(fromType, fromColor, fromTypeBB ^ fromToBB);
        m_board.updateOccupiedBB(m_board.getOccupied() ^ fromToBB);
        m_board.updateEmptyBB(~m_board.getOccupied());
    } else {
        Board::PieceColor captureColor = m_board.getPieceColor(toInd);
        Board::PieceType captureType = m_board.getPieceType(toInd);
        uint64_t captureColorBB = m_board.getPieceSet(Board::all, captureColor);
        uint64_t captureTypeBB = m_board.getPieceSet(captureType, captureColor);

        m_board.updateBB(Board::all, fromColor, fromColorBB ^ fromToBB); //update moving piece
        m_board.updateBB(fromType, fromColor, fromTypeBB ^ fromToBB);
        m_board.updateBB(Board::all, captureColor, captureColorBB ^ toBB); //remove captured piece
        m_board.updateBB(captureType, captureColor, captureTypeBB ^ toBB);
        m_board.updateOccupiedBB(m_board.getOccupied() ^ fromBB);
        m_board.updateEmptyBB(~m_board.getOccupied());
    }
    
    if(flag != EP_CAPTURE) flag &= ~CAPTURE; //consider promotions and promo captures in the same case
    switch(flag) {
        case DOUBLE_PAWN_PUSH:
        {
            //horizontal pin test of possible capturing pawns
            uint64_t epTarget = Board::pawnShift(toBB, Board::getPawnDirection(oppColor));
            uint64_t epCapturers = m_board.pawnAttackTargets(epTarget, fromColor) & m_board.getPieceSet(Board::pawns, oppColor);
            uint64_t empty = m_board.getEmpty();
            uint64_t emptyBeforeMove = empty | toBB; //hor pin test needs to happen before double pawn push move

            uint64_t rookLike = m_board.getPieceSet(Board::rooks, fromColor) | m_board.getPieceSet(Board::queens, fromColor);
            uint64_t oppKing = m_board.getPieceSet(Board::king, oppColor);

            uint64_t horInBetween = (Board::eastFill(rookLike, emptyBeforeMove) & Board::westFill(oppKing, emptyBeforeMove))
                | (Board::westFill(rookLike, emptyBeforeMove) & Board::eastFill(oppKing, emptyBeforeMove));
            uint64_t horPinnedMask = Board::nullBoolMask(horInBetween & epCapturers);

            //diaganol pin test of ep target pawn
            uint64_t king = m_board.getPieceSet(Board::king, fromColor);
            uint64_t oppBishopLike = m_board.getPieceSet(Board::bishops, oppColor) | m_board.getPieceSet(Board::queens, oppColor);

            uint64_t diagInBetween = (Board::northEastFill(oppBishopLike, empty) & Board::southWestFill(king, empty))
                | (Board::southWestFill(oppBishopLike, empty) & Board::northEastFill(king, empty));
            uint64_t antiInBetween = (Board::northWestFill(oppBishopLike, empty) & Board::southEastFill(king, empty))
                | (Board::southEastFill(oppBishopLike, empty) & Board::northWestFill(king, empty));
            uint64_t diagPinnedMask = Board::nullBoolMask((diagInBetween | antiInBetween) & toBB);

            uint64_t legalEpMask = horPinnedMask & diagPinnedMask;
            m_board.updateEnPassantTargets(oppColor, epTarget & legalEpMask);
            break;
        }

        case KING_CASTLE:
        {
            break;
        }

        case QUEEN_CASTLE:
        {
            break;
        }

        case EP_CAPTURE:
        {
            uint64_t captureSquare = Board::pawnShift(toBB, Board::getPawnDirection(oppColor));
            Board::PieceType captureType = m_board.getPieceType(Board::serializeSingleBit(captureSquare));
            uint64_t captureTypeSet = m_board.getPieceSet(captureType, oppColor);
            m_board.updateBB(captureType, oppColor, captureTypeSet ^ captureSquare);
            break;
        }

        case KNIGHT_PROMOTION:
        {
            m_board.updateBB(Board::pawns, fromColor, NOT_LAST_RANK);
            uint64_t knightSet = m_board.getPieceSet(Board::knights, fromColor);
            m_board.updateBB(Board::knights, fromColor, knightSet | toBB);
            break;
        }
            
        case BISHOP_PROMOTION:
        {
            m_board.updateBB(Board::pawns, fromColor, NOT_LAST_RANK);
            uint64_t bishopSet = m_board.getPieceSet(Board::bishops, fromColor);
            m_board.updateBB(Board::bishops, fromColor, bishopSet | toBB);
            break;
        }
            
        case ROOK_PROMOTION:
        {
            m_board.updateBB(Board::pawns, fromColor, NOT_LAST_RANK);
            uint64_t rookSet = m_board.getPieceSet(Board::rooks, fromColor);
            m_board.updateBB(Board::rooks, fromColor, rookSet | toBB);
            break;
        }
            
        case QUEEN_PROMOTION:
        {
            m_board.updateBB(Board::pawns, fromColor, NOT_LAST_RANK);
            uint64_t queenSet = m_board.getPieceSet(Board::queens, fromColor);
            m_board.updateBB(Board::queens, fromColor, queenSet | toBB);
            break;
        }

        default:
            break;
    }
}

void GameState::unMakeMove(Move move) {

}

void GameState::playEngineMove() {
    const Move topMove = m_engine.getTopMove(m_turn);
    makeMove(topMove);
    switchTurn();
}

void GameState::handleClick(int square) {
    for(const Move move : m_currLegalMoves) {
        if(m_selectedSquare == move.getFrom() && square == move.getTo()) {
            // std::cout << "From: " << move.getFrom() << " To: " << move.getTo() << " Flag: " << move.getFlag() << '\n';
            m_selectedSquare = UNDEFINED_SQUARE;
            makeMove(move);
            switchTurn();
            return;
        }
    }

    m_selectedSquare = square;
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

    // std::unordered_map<Board::PieceType, std::pair<PieceColor, PieceType>> pieceTypes { //mapping of board piece type to pair of piece color and type
    //     {{Board::whitePawns, {PieceColor::LIGHT, PieceType::PAWN}}, {Board::blackPawns, {PieceColor::DARK, PieceType::PAWN}}, {Board::whiteKnights, {PieceColor::LIGHT, PieceType::KNIGHT}},
    //     {Board::blackKnights, {PieceColor::DARK, PieceType::KNIGHT}}, {Board::whiteBishops, {PieceColor::LIGHT, PieceType::BISHOP}}, {Board::blackBishops, {PieceColor::DARK, PieceType::BISHOP}},
    //     {Board::whiteRooks, {PieceColor::LIGHT, PieceType::ROOK}}, {Board::blackRooks, {PieceColor::DARK, PieceType::ROOK}}, {Board::whiteQueens, {PieceColor::LIGHT, PieceType::QUEEN}},
    //     {Board::blackQueens, {PieceColor::DARK, PieceType::QUEEN}}, {Board::whiteKing, {PieceColor::LIGHT, PieceType::KING}}, {Board::blackKing, {PieceColor::DARK, PieceType::KING}}}
    // };

    for(int pieceColor = Board::white; pieceColor <= Board::black; ++pieceColor) {
        for(int pieceType = Board::pawns; pieceType <= Board::king; ++pieceType) {
        Board::PieceType type = static_cast<Board::PieceType>(pieceType);
        Board::PieceColor color = static_cast<Board::PieceColor>(pieceColor);
        uint16_t numPieces = Board::serializeBitboard(m_board.getPieceSet(type, color), indBuf);

        appendPieces(pieceList, indBuf, numPieces, color, type);
    }
    }

    return pieceList;
}