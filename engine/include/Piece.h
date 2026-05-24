#pragma once

#include "Board.h"

#include <iostream>
#include <string>

class Piece {
private:
    const Board::PieceColor m_color;
    const Board::PieceType m_type;

    const int m_row;
    const int m_col;

    const std::string path;
    std::string initPath() {
        std::string colorStr = m_color == Board::white ? "l" : "d";
        std::string typeStr;
        switch(m_type) {
            case Board::pawns:
                typeStr = "p";
                break;
            case Board::rooks:
                typeStr = "r";
                break;
            case Board::knights:
                typeStr = "n";
                break;
            case Board::bishops:
                typeStr = "b";
                break;
            case Board::queens:
                typeStr = "q";
                break;
            case Board::king:
                typeStr = "k";
                break;
        }

        return "Chess_" + typeStr + colorStr + "t60.png";
    }

public:
    Piece(const Board::PieceColor& color, const Board::PieceType& type, int row, int col) : m_color{color}, m_type{type}, path{initPath()}, m_row{row}, m_col{col} { }

    const Board::PieceColor& getColor() const { return m_color; }
    const Board::PieceType& getType() const { return m_type; }
    const std::string& getPath() const { return path; }

    const int getRow() const { return m_row; }
    const int getCol() const { return m_col; }
};