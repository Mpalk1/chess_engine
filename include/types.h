#pragma once
#include <cstdint>
#include <string>

typedef uint64_t u64;
typedef uint8_t u8;

enum class Color
{
  none,
  white,
  black
};

enum class PieceType
{
  white_pawn = 0,
  white_knight,
  white_bishop,
  white_rook,
  white_queen,
  white_king,
  black_pawn,
  black_knight,
  black_bishop,
  black_rook,
  black_queen,
  black_king,
  none = 255,
};

constexpr int piece_val(PieceType p) { return static_cast<int>(p); }

enum class MoveType
{
  normal,
  capture,
  double_pawn_push,
  en_passant,
  kingside_castle,
  queenside_castle,
  promotion_queen,
  promotion_rook,
  promotion_bishop,
  promotion_knight,
  promotion_queen_capture,
  promotion_rook_capture,
  promotion_bishop_capture,
  promotion_knight_capture,
};

enum class MoveFlag : u8
{
  none = 0,
  check = 1 << 0,
  double_check = 1 << 1,
  checkmate = 1 << 2,
  stalemate = 1 << 3,
  fork = 1 << 4,
  pin = 1 << 5,
  skewer = 1 << 6,
  discovered_check = 1 << 7,
};

inline MoveFlag operator|(MoveFlag a, MoveFlag b)
{
  return static_cast<MoveFlag>(static_cast<u8>(a) | static_cast<u8>(b));
}
inline MoveFlag operator&(MoveFlag a, MoveFlag b)
{
  return static_cast<MoveFlag>(static_cast<u8>(a) & static_cast<u8>(b));
}
inline MoveFlag &operator|=(MoveFlag &a, MoveFlag b) { return a = a | b; }
inline bool has_flag(MoveFlag flags, MoveFlag query) { return (flags & query) != MoveFlag::none; }


enum class Square : u8
{
  A1 = 0,
  B1,
  C1,
  D1,
  E1,
  F1,
  G1,
  H1,
  A2 = 8,
  B2,
  C2,
  D2,
  E2,
  F2,
  G2,
  H2,
  A3 = 16,
  B3,
  C3,
  D3,
  E3,
  F3,
  G3,
  H3,
  A4 = 24,
  B4,
  C4,
  D4,
  E4,
  F4,
  G4,
  H4,
  A5 = 32,
  B5,
  C5,
  D5,
  E5,
  F5,
  G5,
  H5,
  A6 = 40,
  B6,
  C6,
  D6,
  E6,
  F6,
  G6,
  H6,
  A7 = 48,
  B7,
  C7,
  D7,
  E7,
  F7,
  G7,
  H7,
  A8 = 56,
  B8,
  C8,
  D8,
  E8,
  F8,
  G8,
  H8,
  none = 255
};


inline int rank(Square s) { return static_cast<u8>(s) >> 3; } // 0-7
inline int file(Square s) { return static_cast<u8>(s) & 7; } // 0-7
inline Square make_square(const int file, const int rank) { return static_cast<Square>(rank * 8 + file); }
inline Square make_square(u64 bb)
{
  if (bb == 0)
    return Square::none; // no bits set
  return static_cast<Square>(std::countr_zero(bb));
}

inline std::string square_to_string(Square s)
{
  if (s == Square::none)
    return "none";
  std::string str{};
  str += static_cast<char>('a' + file(s));
  str += static_cast<char>('1' + rank(s));
  return str;
}

// Files
constexpr u64 FILE_A = 0x0101010101010101ULL;
constexpr u64 FILE_B = FILE_A << 1;
constexpr u64 FILE_C = FILE_A << 2;
constexpr u64 FILE_D = FILE_A << 3;
constexpr u64 FILE_E = FILE_A << 4;
constexpr u64 FILE_F = FILE_A << 5;
constexpr u64 FILE_G = FILE_A << 6;
constexpr u64 FILE_H = FILE_A << 7;

// Ranks
constexpr u64 RANK_1 = 0xFFULL;
constexpr u64 RANK_2 = RANK_1 << 8;
constexpr u64 RANK_3 = RANK_1 << 16;
constexpr u64 RANK_4 = RANK_1 << 24;
constexpr u64 RANK_5 = RANK_1 << 32;
constexpr u64 RANK_6 = RANK_1 << 40;
constexpr u64 RANK_7 = RANK_1 << 48;
constexpr u64 RANK_8 = RANK_1 << 56;
