#pragma once
#include <array>
#include "move.h"

struct Position;


struct MoveList
{
  static constexpr int MaxMoves{256};
  std::array<Move, 256> moves{};
  size_t count{};

  void add(Square from, Square to, PieceType piece, MoveType type, PieceType captured,
           u8 castling_rights, Square en_passant_sq, u8 halfmove_clock);
  void add_move(const Move &move) { moves[count++] = move; }
  // void add_pawn_move(const u64 targets, const int advance, const PieceType piece, const MoveType type);
  void add_moves(Square from, u64 targets, PieceType piece,
         const Position& position, u8 castling_rights, Square en_passant_sq, u8 halfmove_clock);
  void clear() {
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
  const Move* begin() const { return &moves[0]; }
  const Move* end() const { return &moves[count]; }
  Move& operator[](int idx) { return moves[idx]; }
  const Move& operator[](int idx) const { return moves[idx]; }
};
