#pragma once
#include <array>
#include <bit>
#include <iostream>

#include "move.h"


struct MoveList
{
  static constexpr int MaxMoves{256};
  std::array<Move, 256> moves{};
  size_t count{};

  void add_move(const Move& move) { moves[count++] = move; }
  void add_pawn_move(const u64 targets, const int advance, const PieceType piece, const MoveType type)
  {
    for (int to = 0; to < 64; ++to)
    {
      if ((targets & (1ULL << to)) == 0)
      {
        continue;
      }

      const int from = to - advance;
      add_move(Move(static_cast<Square>(from), static_cast<Square>(to), piece, type));
    }
  }
  void add_piece_moves(Square from, u64 targets, PieceType piece)
  {
    while (targets) {
      const int to = std::countr_zero(targets);
      add_move(Move(from, static_cast<Square>(to), piece, MoveType::normal));
      targets &= targets - 1;
    }
  }
  void clear()
  {
    count = 0;
    moves.fill(Move());
  }
  void print() const
  {
    for (const auto& move : moves)
    {
      move.print();
    }
  }
  const Move& begin() const { return moves[0]; }
  const Move& end() const { return moves[count]; }
  Move& operator[](int idx) { return moves[idx]; }
};
