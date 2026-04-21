#pragma once
#include <string>

#include "board_list.h"
#include "move.h"
#include "move_list.h"
#include "types.h"

struct Position
{

	BoardList bitboards{};
	MoveList move_list{};
	const std::string starting_fen{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
	Color current_turn{ Color::white };
	u8 castling_rights{ castle_white_kingside | castle_white_queenside | castle_black_kingside | castle_black_queenside };
	Square en_passant_square{ Square::none };
	u8 halfmove_clock{ 0 };
	int fullmove_number{ 1 };
	std::array<PieceType, 64> mailbox{};
	u64 zobrist_key{ 0 };

	u64 get_empty_squares() const { return bitboards.empty(); }
	Position();
	void clear();
	void read_fen(const std::string& fen);
	void apply_move(const Move& move, MoveState& state);
	void undo_move(const Move& move, const MoveState& state);
	void make_move(const std::string& token);
	PieceType piece_at(Square sq) const;
	MoveList& get_legal_moves();
	MoveList& get_pseudo_legal_moves();
	u64 get_squares(Color color) const;
	void print() const;
	void print_moves() const { move_list.print(); }

	static constexpr u8 castle_white_kingside = 1 << 0;
	static constexpr u8 castle_white_queenside = 1 << 1;
	static constexpr u8 castle_black_kingside = 1 << 2;
	static constexpr u8 castle_black_queenside = 1 << 3;
};


bool is_number(char c);

/* bitboards mapping -> eg. bit 10 is C2
	A  B  C  D  E  F  G  H
8| 56 57 58 59 60 61 62 63
7| 48 49 50 51 52 53 54 55
6| 40 41 42 43 44 45 46 47
5| 32 33 34 35 36 37 38 39
4| 24 25 26 27 28 29 30 31
3| 16 17 18 19 20 21 22 23
2|  8  9 10 11 12 13 14 15
1|  0  1  2  3  4  5  6  7
 */ //every bitboard represents one type of piece black or white so
 //    bitboards[0] represents all the white pawns
