#include "move_list.h"
#include "types.h"

void MoveList::add_piece_moves(Square from, u64 targets, PieceType piece)
{
  while (targets)
  {
    const int to = std::countr_zero(targets);
    add_move(Move(from, static_cast<Square>(to), piece, MoveType::normal));
    targets &= targets - 1;
  }
}

void MoveList::add_pawn_move(const u64 targets, const int advance, const PieceType piece, const MoveType type)
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

void MoveList::add(u64 before, u64 after, const PieceType piece_type, const MoveType move_type)
{
  add_move(Move{make_square(before), make_square(after), piece_type, move_type});
}
