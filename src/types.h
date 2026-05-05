#pragma once

#include <bit>
#include <cstdint>
#include <string>

using u64 = uint64_t;
using u8 = uint8_t;

enum class Color
{
    None,
    White,
    Black
};

enum class PieceType
{
    WhitePawn = 0,
    WhiteKnight,
    WhiteBishop,
    WhiteRook,
    WhiteQueen,
    WhiteKing,
    BlackPawn,
    BlackKnight,
    BlackBishop,
    BlackRook,
    BlackQueen,
    BlackKing,
    None = 255,
};

constexpr int piece_val(PieceType p)
{
    return static_cast<int>(p);
}

enum class MoveType
{
    Normal,
    Capture,
    DoublePawnPush,
    EnPassant,
    KingsideCastle,
    QueensideCastle,
    PromotionQueen,
    PromotionRook,
    PromotionBishop,
    PromotionKnight,
    PromotionQueenCapture,
    PromotionRookCapture,
    PromotionBishopCapture,
    PromotionKnightCapture,
};

enum class MoveFlag : u8
{
    None = 0,
    Check = 1 << 0,
    DoubleCheck = 1 << 1,
    Checkmate = 1 << 2,
    Stalemate = 1 << 3,
    Fork = 1 << 4,
    Pin = 1 << 5,
    Skewer = 1 << 6,
    DiscoveredCheck = 1 << 7,
};

inline MoveFlag operator|(MoveFlag a, MoveFlag b)
{
    return static_cast<MoveFlag>(static_cast<u8>(a) | static_cast<u8>(b));
}
inline MoveFlag operator&(MoveFlag a, MoveFlag b)
{
    return static_cast<MoveFlag>(static_cast<u8>(a) & static_cast<u8>(b));
}
inline MoveFlag& operator|=(MoveFlag& a, MoveFlag b)
{
    return a = a | b;
}
inline bool has_flag(MoveFlag flags, MoveFlag query)
{
    return (flags & query) != MoveFlag::None;
}

enum class Square : u8
{
    A1 = 0, B1, C1, D1, E1, F1, G1, H1,
    A2 = 8, B2, C2, D2, E2, F2, G2, H2,
    A3 = 16, B3, C3, D3, E3, F3, G3, H3,
    A4 = 24, B4, C4, D4, E4, F4, G4, H4,
    A5 = 32, B5, C5, D5, E5, F5, G5, H5,
    A6 = 40, B6, C6, D6, E6, F6, G6, H6,
    A7 = 48, B7, C7, D7, E7, F7, G7, H7,
    A8 = 56, B8, C8, D8, E8, F8, G8, H8,
    None = 255
};

inline int rank(Square s)
{
    return static_cast<u8>(s) >> 3;
} 
inline int file(Square s)
{
    return static_cast<u8>(s) & 7;
} 
inline Square make_square(const int file, const int rank)
{
    return static_cast<Square>(rank * 8 + file);
}
inline Square make_square(int sq)
{
    if (sq < 0 || sq >= 64)
        return Square::None;
    return static_cast<Square>(sq);
}
inline Square make_square(u64 bb)
{
    if (bb == 0)
        return Square::None; 
    return static_cast<Square>(std::countr_zero(bb));
}

inline Square square_from_chars(char file, char rank)
{
    file = static_cast<char>(std::tolower(static_cast<unsigned char>(file)));
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8')
        return Square::None;
    return make_square(file - 'a', rank - '1');
}

inline Square square_from_string(std::string_view s)
{
    if (s.size() != 2)
        return Square::None;
    return square_from_chars(s[0], s[1]);
}

inline std::string square_to_string(Square s)
{
    if (s == Square::None)
        return "none";
    std::string str{};
    str += static_cast<char>('a' + file(s));
    str += static_cast<char>('1' + rank(s));
    return str;
}

struct GameState
{
    u8 castling_rights{0};
    Square en_passant_square{Square::None};
    u8 halfmove_clock{0};
    int fullmove_number{1};
    Color turn{Color::White};
    u64 zobrist_key{0};
};


constexpr u64 FILE_A = 0x0101010101010101ULL;
constexpr u64 FILE_B = FILE_A << 1;
constexpr u64 FILE_C = FILE_A << 2;
constexpr u64 FILE_D = FILE_A << 3;
constexpr u64 FILE_E = FILE_A << 4;
constexpr u64 FILE_F = FILE_A << 5;
constexpr u64 FILE_G = FILE_A << 6;
constexpr u64 FILE_H = FILE_A << 7;


constexpr u64 RANK_1 = 0xFFULL;
constexpr u64 RANK_2 = RANK_1 << 8;
constexpr u64 RANK_3 = RANK_1 << 16;
constexpr u64 RANK_4 = RANK_1 << 24;
constexpr u64 RANK_5 = RANK_1 << 32;
constexpr u64 RANK_6 = RANK_1 << 40;
constexpr u64 RANK_7 = RANK_1 << 48;
constexpr u64 RANK_8 = RANK_1 << 56;


constexpr u8 CASTLE_WHITE_KINGSIDE = 1 << 0;
constexpr u8 CASTLE_WHITE_QUEENSIDE = 1 << 1;
constexpr u8 CASTLE_BLACK_KINGSIDE = 1 << 2;
constexpr u8 CASTLE_BLACK_QUEENSIDE = 1 << 3;
