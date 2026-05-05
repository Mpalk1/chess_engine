#pragma once

#include "types.h"
#include <array>

namespace Evaluation
{
constexpr std::array<int, 12> PIECE_VALUES{100, 320, 330, 500, 900, 0,
                                           100, 320, 330, 500, 900, 0};

constexpr std::array<int, 64> mirror_table(const std::array<int, 64>& table)
{
    std::array<int, 64> mirrored{};
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            mirrored[r * 8 + c] = table[(7 - r) * 8 + c];
    return mirrored;
}

constexpr std::array<int, 64> WHITE_PAWN_TABLE{
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5, 5, 10, 25, 25, 10, 5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0};

constexpr std::array<int, 64> BLACK_PAWN_TABLE = mirror_table(WHITE_PAWN_TABLE);

constexpr std::array<int, 64> WHITE_KNIGHT_TABLE{
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50};

constexpr std::array<int, 64> BLACK_KNIGHT_TABLE = mirror_table(WHITE_KNIGHT_TABLE);

constexpr std::array<int, 64> WHITE_BISHOP_TABLE{
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20};

constexpr std::array<int, 64> BLACK_BISHOP_TABLE = mirror_table(WHITE_BISHOP_TABLE);

constexpr std::array<int, 64> WHITE_ROOK_TABLE{
    0, 0, 0, 5, 5, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    5, 10, 10, 10, 10, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0};

constexpr std::array<int, 64> BLACK_ROOK_TABLE = mirror_table(WHITE_ROOK_TABLE);

constexpr std::array<int, 64> WHITE_QUEEN_TABLE{
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20};

constexpr std::array<int, 64> BLACK_QUEEN_TABLE = mirror_table(WHITE_QUEEN_TABLE);

constexpr std::array<int, 64> WHITE_KING_TABLE{
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20, 0, 0, 0, 0, 20, 20,
    20, 30, 10, 0, 0, 10, 30, 20};

constexpr std::array<int, 64> BLACK_KING_TABLE = mirror_table(WHITE_KING_TABLE);

constexpr std::array<std::array<int, 64>, 12> PIECE_SQUARE_TABLES{
    WHITE_PAWN_TABLE,
    WHITE_KNIGHT_TABLE,
    WHITE_BISHOP_TABLE,
    WHITE_ROOK_TABLE,
    WHITE_QUEEN_TABLE,
    WHITE_KING_TABLE,
    BLACK_PAWN_TABLE,
    BLACK_KNIGHT_TABLE,
    BLACK_BISHOP_TABLE,
    BLACK_ROOK_TABLE,
    BLACK_QUEEN_TABLE,
    BLACK_KING_TABLE,
};
} 
