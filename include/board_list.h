#pragma once
#include "bitboard.h"
#include <array>

struct BoardList
{
  std::array<Bitboard, 12> bitboards{};

  Bitboard& operator[](PieceType piece);
  const Bitboard& operator[](PieceType piece) const;
  const Bitboard& operator[](int idx) const;

  void clear();
  u64 occupied() const;
  u64 occupied(Color color) const;
  u64 empty() const;
  PieceType piece_at(Square s) const;

};
