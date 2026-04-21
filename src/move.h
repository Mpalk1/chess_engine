#pragma once
#include <iostream>
#include "types.h"

struct MoveState
{
	u8 castling_rights{0};
	Square enpassant_sq{Square::none};
	u8 halfmove_clock{0};
	int fullmove_number{1};
	Color turn{Color::white};
};

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

        Move()
            : from(Square::A1), to(Square::A1), piece(PieceType::none),
              captured(PieceType::none), type(MoveType::normal),
		      flags(MoveFlag::none), castling_rights(0),
		      enpassant_sq(Square::none), halfmove_clock(0) {
	}

	Move(Square from, Square to, PieceType piece, MoveType type = MoveType::normal) :
		from(from), to(to), piece(piece), captured(PieceType::none), type(type), flags(MoveFlag::none),
		castling_rights(0), enpassant_sq(Square::none), halfmove_clock(0)
	{}

	Move(Square from, Square to, PieceType piece, MoveType type, PieceType captured,
		u8 castling_rights, Square en_passant_sq, u8 halfmove_clock) :
		from(from), to(to), piece(piece), captured(captured), type(type), flags(MoveFlag::none),
		castling_rights(castling_rights), enpassant_sq(en_passant_sq), halfmove_clock(halfmove_clock)
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

inline char promotion_suffix(MoveType type)
{
	switch (type)
	{
	case MoveType::promotion_queen:
	case MoveType::promotion_queen_capture:
		return 'q';
	case MoveType::promotion_rook:
	case MoveType::promotion_rook_capture:
		return 'r';
	case MoveType::promotion_bishop:
	case MoveType::promotion_bishop_capture:
		return 'b';
	case MoveType::promotion_knight:
	case MoveType::promotion_knight_capture:
		return 'n';
	default:
		return '\0';
	}
}

inline std::string move_to_uci(const Move& move)
{
	std::string uci = square_to_string(move.from) + square_to_string(move.to);
	if (const char suffix = promotion_suffix(move.type); suffix != '\0')
		uci.push_back(suffix);
	return uci;
}
