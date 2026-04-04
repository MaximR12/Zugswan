#include "GameState.h"
#include <unordered_map>

void GameState::makeMove(Move move) {
    uint16_t fromInd = move.getFrom(), toInd = move.getTo();
    uint64_t fromBB = 1ULL << fromInd;
    uint64_t toBB = 1ULL << toInd; 
    uint64_t fromToBB = fromBB ^ toBB;

    Board::PieceType fromColor = m_board.getPieceColor(fromInd);
    Board::PieceType fromType = m_board.getPieceType(fromInd);
    uint64_t fromColorBB = m_board.getPieceSet(fromColor);
    uint64_t fromTypeBB = m_board.getPieceSet(fromType);

    m_board.updateOccupiedBB(m_board.getOccupied() ^ fromToBB);
    m_board.updateEmptyBB(~m_board.getOccupied());

    if(!move.isCapture()) {
        m_board.updateBB(fromColor, fromColorBB ^ fromToBB);
        m_board.updateBB(fromType, fromTypeBB ^ fromToBB);
    } else {
        Board::PieceType captureColor = m_board.getPieceColor(toInd);
        Board::PieceType captureType = m_board.getPieceType(toInd);
        uint64_t captureColorBB = m_board.getPieceSet(captureColor);
        uint64_t captureTypeBB = m_board.getPieceSet(captureType);

        m_board.updateBB(fromColor, fromColorBB ^ fromToBB); //update moving piece
        m_board.updateBB(fromType, fromTypeBB ^ fromToBB);
        m_board.updateBB(captureColor, captureColorBB ^ toBB); //remove captured piece
        m_board.updateBB(captureType, captureTypeBB ^ toBB);
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

void appendPieces(std::vector<Piece>& pieceList, std::array<uint16_t, NUM_SQUARES>& indBuf, uint16_t numPieces, const PieceColor& color, const PieceType& type) {
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

    std::unordered_map<Board::PieceType, std::pair<PieceColor, PieceType>> pieceTypes { //mapping of board piece type to pair of piece color and type
        {{Board::whitePawns, {PieceColor::LIGHT, PieceType::PAWN}}, {Board::blackPawns, {PieceColor::DARK, PieceType::PAWN}}, {Board::whiteKnights, {PieceColor::LIGHT, PieceType::KNIGHT}},
        {Board::blackKnights, {PieceColor::DARK, PieceType::KNIGHT}}, {Board::whiteBishops, {PieceColor::LIGHT, PieceType::BISHOP}}, {Board::blackBishops, {PieceColor::DARK, PieceType::BISHOP}},
        {Board::whiteRooks, {PieceColor::LIGHT, PieceType::ROOK}}, {Board::blackRooks, {PieceColor::DARK, PieceType::ROOK}}, {Board::whiteQueens, {PieceColor::LIGHT, PieceType::QUEEN}},
        {Board::blackQueens, {PieceColor::DARK, PieceType::QUEEN}}, {Board::whiteKing, {PieceColor::LIGHT, PieceType::KING}}, {Board::blackKing, {PieceColor::DARK, PieceType::KING}}}
    };

    for(int pieceType = Board::whitePawns; pieceType <= Board::blackKing; ++pieceType) {
        uint16_t numPieces = Board::serializeBitboard(m_board.getPieceSet(static_cast<Board::PieceType>(pieceType)), indBuf);
        const auto &[color, type] = pieceTypes[static_cast<Board::PieceType>(pieceType)];
        appendPieces(pieceList, indBuf, numPieces, color, type);
    }

    return pieceList;
}