#pragma once
#include <cstdint>

typedef uint64_t u64;
typedef uint8_t  u8;

enum class Color {
  none,
  white,
  black
};

enum class PieceType {
  none = 0,
  white_pawn = 1,
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
};

enum class MoveType {
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

enum class MoveFlag : u8 {
  none             = 0,
  check            = 1 << 0,
  double_check     = 1 << 1,
  checkmate        = 1 << 2,
  stalemate        = 1 << 3,
  fork             = 1 << 4,
  pin              = 1 << 5,
  skewer           = 1 << 6,
  discovered_check = 1 << 7,
};

inline MoveFlag operator|(MoveFlag a, MoveFlag b) {
  return static_cast<MoveFlag>(static_cast<u8>(a) | static_cast<u8>(b));
}
inline MoveFlag operator&(MoveFlag a, MoveFlag b) {
  return static_cast<MoveFlag>(static_cast<u8>(a) & static_cast<u8>(b));
}
inline MoveFlag& operator|=(MoveFlag& a, MoveFlag b) {
  return a = a | b;
}
inline bool has_flag(MoveFlag flags, MoveFlag query) {
  return (flags & query) != MoveFlag::none;
}


enum class Square : u8 {
  A1=0,  B1,  C1,  D1,  E1,  F1,  G1,  H1,
  A2=8,  B2,  C2,  D2,  E2,  F2,  G2,  H2,
  A3=16, B3,  C3,  D3,  E3,  F3,  G3,  H3,
  A4=24, B4,  C4,  D4,  E4,  F4,  G4,  H4,
  A5=32, B5,  C5,  D5,  E5,  F5,  G5,  H5,
  A6=40, B6,  C6,  D6,  E6,  F6,  G6,  H6,
  A7=48, B7,  C7,  D7,  E7,  F7,  G7,  H7,
  A8=56, B8,  C8,  D8,  E8,  F8,  G8,  H8,
  none=255
};


inline int rank(Square s) { return static_cast<u8>(s) >> 3; }  // 0-7
inline int file(Square s) { return static_cast<u8>(s) & 7;  }  // 0-7
inline Square make_square(const int file, const int rank) {
  return static_cast<Square>(rank * 8 + file);
}