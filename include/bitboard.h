#pragma once
#include "types.h"

enum class Direction
{
  north = 8,
  south = -north,
  east = 1,
  west = -east,
  north_east = 9,
  north_west = 7,
  south_east = -north_west,
  south_west = -north_east
};

struct Bitboard
{
  u64 bitboard{0ULL};

  Bitboard() = default;
  explicit Bitboard(u64 b) : bitboard(b) {}
  u64 get() const;
  Bitboard shift(Direction dir, int times) const;
  void shift_inplace(Direction dir, int times);
  void print_as_bits() const;
  void is_bit_at(const u64& x) const;
  void is_bit_at(const Square& square) const;
  void clear();

  Bitboard& operator<<=(int times);
  Bitboard& operator>>=(int times);
  Bitboard operator<<(int times) const;
  Bitboard operator>>(int times) const;

  Bitboard& operator&=(u64 rhs);
  Bitboard& operator|=(u64 rhs);
  Bitboard& operator^=(u64 rhs);
  Bitboard operator&(u64 rhs) const;
  Bitboard operator|(u64 rhs) const;
  Bitboard operator^(u64 rhs) const;

  Bitboard& operator&=(const Bitboard& rhs);
  Bitboard& operator|=(const Bitboard& rhs);
  Bitboard& operator^=(const Bitboard& rhs);
  Bitboard operator&(const Bitboard& rhs) const;
  Bitboard operator|(const Bitboard& rhs) const;
  Bitboard operator^(const Bitboard& rhs) const;

  bool operator==(const Bitboard& rhs) const;
  bool operator!=(const Bitboard& rhs) const;
  explicit operator bool() const;
  Bitboard operator~() const;


};

