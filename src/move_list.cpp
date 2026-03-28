#include "move_list.h"
#include <bit>
#include "types.h"


void MoveList::add_moves(Square from, u64 targets, PieceType piece, const BoardList& board, u8 castling_rights,
	Square en_passant_sq, u8 halfmove_clock)
{
	while (targets)
	{
		const int to_idx = std::countr_zero(targets);
		const auto to = make_square(to_idx);
		const auto captured = board.piece_at(to);
		const auto type = (captured != PieceType::none) ? MoveType::capture : MoveType::normal;

		// If a rook is captured on its starting square, remove the corresponding castling right
		u8 new_castling = castling_rights;
		if (to_idx == 7)  new_castling &= ~CASTLE_WHITE_KINGSIDE;   // H1
		else if (to_idx == 0)  new_castling &= ~CASTLE_WHITE_QUEENSIDE;  // A1
		else if (to_idx == 63) new_castling &= ~CASTLE_BLACK_KINGSIDE;   // H8
		else if (to_idx == 56) new_castling &= ~CASTLE_BLACK_QUEENSIDE;  // A8

		const u8 new_halfmove = (captured != PieceType::none) ? 0 : static_cast<u8>(halfmove_clock + 1);

		add_move(Move{ from, to, piece, type, captured, new_castling, en_passant_sq, new_halfmove });
		targets &= targets - 1;
	}
}
void MoveList::add(Square from, Square to, PieceType piece, MoveType type, PieceType captured, u8 castling_rights,
	Square en_passant_sq, u8 halfmove_clock)
{
	add_move(Move{ from, to, piece, type, captured, castling_rights, en_passant_sq, halfmove_clock });
}
