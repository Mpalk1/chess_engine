#include "move_list.h"
#include <bit>
#include "types.h"


void MoveList::add_moves(Square from, u64 targets, PieceType piece, const BoardList &board, u8 castling_rights,
                         Square en_passant_sq, u8 halfmove_clock)
{
  while (targets)
  {
    const auto to_idx = std::countr_zero(targets);
    const auto to = make_square(to_idx);
    const auto captured = board.piece_at(to);
    const auto type = (captured != PieceType::none) ? MoveType::capture : MoveType::normal;
    add_move(Move{from, to, piece, type, captured, castling_rights, en_passant_sq, halfmove_clock});
    targets &= targets - 1;
  }
}
void MoveList::add(Square from, Square to, PieceType piece, MoveType type, PieceType captured, u8 castling_rights,
                   Square en_passant_sq, u8 halfmove_clock)
{
  add_move(Move{from, to, piece, type, captured, castling_rights, en_passant_sq, halfmove_clock});
}
