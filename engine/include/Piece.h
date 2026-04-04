#pragma once

#include <iostream>
#include <string>

enum class PieceColor {
    LIGHT, DARK
};

enum class PieceType {
    PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING
};

class Piece {
private:
    const PieceColor m_color;
    const PieceType m_type;

    const int m_row;
    const int m_col;

    const std::string path;
    std::string initPath() {
        std::string colorStr = m_color == PieceColor::LIGHT ? "l" : "d";
        std::string typeStr;
        switch(m_type) {
            case PieceType::PAWN:
                typeStr = "p";
                break;
            case PieceType::ROOK:
                typeStr = "r";
                break;
            case PieceType::KNIGHT:
                typeStr = "n";
                break;
            case PieceType::BISHOP:
                typeStr = "b";
                break;
            case PieceType::QUEEN:
                typeStr = "q";
                break;
            case PieceType::KING:
                typeStr = "k";
                break;
        }

        return "Chess_" + typeStr + colorStr + "t60.png";
    }

public:
    Piece(const PieceColor& color, const PieceType& type, int row, int col) : m_color{color}, m_type{type}, path{initPath()}, m_row{row}, m_col{col} { }

    const PieceColor& getColor() const { return m_color; }
    const PieceType& getType() const { return m_type; }
    const std::string& getPath() const { return path; }

    const int getRow() const { return m_row; }
    const int getCol() const { return m_col; }
};