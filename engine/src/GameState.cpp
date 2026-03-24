#include "GameState.h"
#include <unordered_map>

using namespace Game;

void GameState::makeMove(Move move) {
    int from = move.getFrom();
    int to = move.getTo();
}

void GameState::unMakeMove(Move move) {
    
}

void GameState::handleClick(int row, int col) {

}

void appendPieces(std::vector<Piece>& pieceList, Game::IndArr& arr, const PieceColor& color, const PieceType& type) {
    for(int i = 0; i < arr.size; ++i) {
        int ind = arr[i];
        int row = ind / 8, col = ind % 8;
        pieceList.emplace_back(color, type, row, col);
    }
}

std::vector<Piece> GameState::getPieceList() const {
    std::vector<Piece> pieceList;
    pieceList.reserve(NUM_SQUARES);

    IndArr wPawnInds, bPawnInds, wKnightInds, bKnightInds, wBishopInds, bBishopInds, wRookInds, bRookInds, wQueenInds, bQueenInds, wKingInd, bKingInd;
    std::array<std::pair<IndArr&, Board::PieceType>, 12> pieceSets { //List of indice array, piece type pairings
        {{wPawnInds, Board::whitePawns}, {bPawnInds, Board::blackPawns}, {wKnightInds, Board::whiteKnights}, {bKnightInds, Board::blackKnights},
        {wBishopInds, Board::whiteBishops}, {bBishopInds, Board::blackBishops}, {wRookInds, Board::whiteRooks}, {bRookInds, Board::blackRooks},
        {wQueenInds, Board::whiteQueens}, {bQueenInds, Board::blackQueens}, {wKingInd, Board::whiteKing}, {bKingInd, Board::blackKing}}
    };

    std::unordered_map<Board::PieceType, std::pair<PieceColor, PieceType>> pieceTypes { //mapping of board piece type to pair of piece color and type
        {{Board::whitePawns, {PieceColor::LIGHT, PieceType::PAWN}}, {Board::blackPawns, {PieceColor::DARK, PieceType::PAWN}}, {Board::whiteKnights, {PieceColor::LIGHT, PieceType::KNIGHT}},
        {Board::blackKnights, {PieceColor::DARK, PieceType::KNIGHT}}, {Board::whiteBishops, {PieceColor::LIGHT, PieceType::BISHOP}}, {Board::blackBishops, {PieceColor::DARK, PieceType::BISHOP}},
        {Board::whiteRooks, {PieceColor::LIGHT, PieceType::ROOK}}, {Board::blackRooks, {PieceColor::DARK, PieceType::ROOK}}, {Board::whiteQueens, {PieceColor::LIGHT, PieceType::QUEEN}},
        {Board::blackQueens, {PieceColor::DARK, PieceType::QUEEN}}, {Board::whiteKing, {PieceColor::LIGHT, PieceType::KING}}, {Board::blackKing, {PieceColor::DARK, PieceType::KING}}}
    };

    for(const auto &[indArr, pieceType] : pieceSets) {
        Board::serializeBitboard(m_board.getPieceSet(pieceType), indArr);
        const auto &[color, type] = pieceTypes[pieceType];
        appendPieces(pieceList, indArr, color, type);
    }

    return pieceList;
}