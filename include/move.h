#pragma once
#include <iostream>
#include "types.h"

struct Move
{
	Square from; // square 0-63
	Square to; // square 0-63
	PieceType piece;
	PieceType captured; // PieceType::none if quiet move
	MoveType type;
	MoveFlag flags;

	u8 castling_rights; // post-move castling rights
	Square enpassant_sq; // post-move en passant square
	u8 halfmove_clock; // post-move halfmove clock

	// board state before this move was made
	u8 prev_castling_rights;
	Square prev_enpassant_sq;
	u8 prev_halfmove_clock;
	int prev_fullmove_number;
	Color prev_turn;

	Move() :
		from(Square::A1), to(Square::A1), piece(PieceType::none), captured(PieceType::none), type(MoveType::normal),
		flags(MoveFlag::none), castling_rights(0), enpassant_sq(Square::none), halfmove_clock(0),
		prev_castling_rights(0), prev_enpassant_sq(Square::none), prev_halfmove_clock(0), prev_fullmove_number(1),
		prev_turn(Color::white)
	{}

	Move(Square from, Square to, PieceType piece, MoveType type = MoveType::normal) :
		from(from), to(to), piece(piece), captured(PieceType::none), type(type), flags(MoveFlag::none),
		castling_rights(0), enpassant_sq(Square::none), halfmove_clock(0),
		prev_castling_rights(0), prev_enpassant_sq(Square::none), prev_halfmove_clock(0), prev_fullmove_number(1),
		prev_turn(Color::white)
	{}

	Move(Square from, Square to, PieceType piece, MoveType type, PieceType captured,
		u8 castling_rights, Square en_passant_sq, u8 halfmove_clock) :
		from(from), to(to), piece(piece), captured(captured), type(type), flags(MoveFlag::none),
		castling_rights(castling_rights), enpassant_sq(en_passant_sq), halfmove_clock(halfmove_clock),
		prev_castling_rights(0), prev_enpassant_sq(Square::none), prev_halfmove_clock(0), prev_fullmove_number(1),
		prev_turn(Color::white)
	{}

	bool is_capture() const { return captured != PieceType::none; }
	bool is_promotion() const
	{
		return type == MoveType::promotion_queen || type == MoveType::promotion_rook ||
			type == MoveType::promotion_bishop || type == MoveType::promotion_knight ||
			type == MoveType::promotion_queen_capture || type == MoveType::promotion_rook_capture ||
			type == MoveType::promotion_bishop_capture || type == MoveType::promotion_knight_capture;
	}
	bool is_castle() const { return type == MoveType::kingside_castle || type == MoveType::queenside_castle; }
	bool is_check() const { return has_flag(flags, MoveFlag::check); }
	bool is_checkmate() const { return has_flag(flags, MoveFlag::checkmate); }
	void print() const
	{
		if (piece == PieceType::none)
		{
			// std::cout << "piece type is none\n";
			return;
		}

		std::string move_str = square_to_string(from) + std::string{ "->" } + square_to_string(to);
		// std::cout << "printing moves\n";
		// if (is_promotion()) {
		//   if (type == MoveType::promotion_queen || type == MoveType::promotion_queen_capture) move_str += "q";
		//   else if (type == MoveType::promotion_rook || type == MoveType::promotion_rook_capture) move_str += "r";
		//   else if (type == MoveType::promotion_bishop || type == MoveType::promotion_bishop_capture) move_str += "b";
		//   else if (type == MoveType::promotion_knight || type == MoveType::promotion_knight_capture) move_str += "n";
		// }

		std::cout << move_str << "\n";
	}
};
