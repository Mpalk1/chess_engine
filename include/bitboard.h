#pragma once
#include "types.h"


enum class Direction
{
  north = 8, south = -north, east = 1, west = -east
};

inline int get_direction(Direction dir)
{
  return static_cast<int>(dir);
}

inline u64 shift(u64 bitboard, Direction dir, int times)
{
  u64 temp{};
  if (const auto d = get_direction(dir); d < 0)
  {
    temp = bitboard >> -d * times;
  }
  else
  {
    temp = bitboard << d * times;
  }
  return temp;
}

inline void print_board(u64 bitboard)
{
  std::cout << std::bitset<64>(bitboard) << "\n";
}