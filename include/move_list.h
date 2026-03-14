#pragma once
#include <array>
#include "move.h"


struct MoveList
{
  static constexpr int MaxMoves{256};
  std::array<Move, 256> moves{};
  size_t count{};

  void add(u64 before, u64 after, const PieceType piece_type, const MoveType move_type);
  void add_move(const Move &move) { moves[count++] = move; }
  void add_pawn_move(const u64 targets, const int advance, const PieceType piece, const MoveType type);
  void add_piece_moves(Square from, u64 targets, PieceType piece);
  void clear()
  {
    count = 0;
    moves.fill(Move());
  }
  void print() const
  {
    for (const auto &move: moves)
    {
      move.print();
    }
  }
  const Move &begin() const { return moves[0]; }
  const Move &end() const { return moves[count]; }
  Move &operator[](int idx) { return moves[idx]; }
};
