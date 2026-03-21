#include "board.h"
#include "bitboard.h"
#include "move_list.h"
#include "types.h"
#include <bit>
#include <cassert>
#include <cctype>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include "tracy/Tracy.hpp"

Board::Board()
{
	ZoneScoped;
	init_knight_attacks();
	init_king_attacks();
	init_ray_attacks();
}

void Board::clear()
{
	ZoneScoped;
	bitboards.clear();
	move_list.clear();
	current_turn = Color::white;
	castling_rights = 0;
	en_passant_square = Square::none;
	halfmove_clock = 0;
	fullmove_number = 1;
	previous_move = Move{};
}

void Board::make_move(Square from, Square to)
{
	ZoneScoped;
	const u64 from_bb = 1ULL << static_cast<int>(from);
	const u64 to_bb = 1ULL << static_cast<int>(to);

	PieceType moving_piece = PieceType::none;
	for (int i = 0; i < 12; ++i)
	{
		if (bitboards[i] & from_bb)
		{
			moving_piece = static_cast<PieceType>(i);
			break;
		}
	}

	if (moving_piece == PieceType::none)
		return;

	for (int i = 0; i < 12; ++i)
	{
		if (bitboards[i] & to_bb)
		{
			bitboards[static_cast<PieceType>(i)] &= ~to_bb;
			break;
		}
	}

	bitboards[moving_piece] &= ~from_bb;
	bitboards[moving_piece] |= to_bb;

	current_turn = (current_turn == Color::white) ? Color::black : Color::white;
}

void Board::unmake_move(Move& move)
{
	ZoneScoped;
	if (move.piece == PieceType::none)
		return;

	const u64 from_bb = 1ULL << static_cast<int>(move.from);
	const u64 to_bb = 1ULL << static_cast<int>(move.to);

	current_turn = move.prev_turn;
	castling_rights = move.prev_castling_rights;
	en_passant_square = move.prev_enpassant_sq;
	halfmove_clock = move.prev_halfmove_clock;
	fullmove_number = move.prev_fullmove_number;

	if (move.type == MoveType::kingside_castle)
	{
		bitboards[move.piece] &= ~to_bb;
		bitboards[move.piece] |= from_bb;

		if (current_turn == Color::white)
		{
			bitboards[PieceType::white_rook] &=
				~(1ULL << static_cast<int>(Square::F1));
			bitboards[PieceType::white_rook] |=
				(1ULL << static_cast<int>(Square::H1));
		}
		else
		{
			bitboards[PieceType::black_rook] &=
				~(1ULL << static_cast<int>(Square::F8));
			bitboards[PieceType::black_rook] |=
				(1ULL << static_cast<int>(Square::H8));
		}
	}
	else if (move.type == MoveType::queenside_castle)
	{
		bitboards[move.piece] &= ~to_bb;
		bitboards[move.piece] |= from_bb;

		if (current_turn == Color::white)
		{
			bitboards[PieceType::white_rook] &=
				~(1ULL << static_cast<int>(Square::D1));
			bitboards[PieceType::white_rook] |=
				(1ULL << static_cast<int>(Square::A1));
		}
		else
		{
			bitboards[PieceType::black_rook] &=
				~(1ULL << static_cast<int>(Square::D8));
			bitboards[PieceType::black_rook] |=
				(1ULL << static_cast<int>(Square::A8));
		}
	}
	else if (move.type == MoveType::en_passant)
	{
		bitboards[move.piece] &= ~to_bb;
		bitboards[move.piece] |= from_bb;

		const int captured_sq = (current_turn == Color::white)
			? static_cast<int>(move.to) - 8
			: static_cast<int>(move.to) + 8;
		bitboards[move.captured] |= (1ULL << captured_sq);
	}
	else if (move.is_promotion())
	{
		const bool is_white = (current_turn == Color::white);
		PieceType promo_piece;
		switch (move.type)
		{
		case MoveType::promotion_queen:
		case MoveType::promotion_queen_capture:
			promo_piece = is_white ? PieceType::white_queen : PieceType::black_queen;
			break;
		case MoveType::promotion_rook:
		case MoveType::promotion_rook_capture:
			promo_piece = is_white ? PieceType::white_rook : PieceType::black_rook;
			break;
		case MoveType::promotion_bishop:
		case MoveType::promotion_bishop_capture:
			promo_piece =
				is_white ? PieceType::white_bishop : PieceType::black_bishop;
			break;
		default:
			promo_piece =
				is_white ? PieceType::white_knight : PieceType::black_knight;
			break;
		}

		bitboards[promo_piece] &= ~to_bb;
		bitboards[move.piece] |= from_bb;

		if (move.captured != PieceType::none)
			bitboards[move.captured] |= to_bb;
	}
	else
	{
		bitboards[move.piece] &= ~to_bb;
		bitboards[move.piece] |= from_bb;

		if (move.captured != PieceType::none)
			bitboards[move.captured] |= to_bb;
	}
}

void Board::unmake_move(Square from, Square to)
{
	ZoneScoped;
	if (previous_move.from == from && previous_move.to == to)
	{
		unmake_move(previous_move);
		previous_move = Move{};
	}
}

MoveList& Board::get_legal_moves()
{
	ZoneScoped;
	auto& moves = get_pseudo_legal_moves();
	const size_t original_count = moves.count;

	auto is_square_attacked_by = [this](Square sq, Color attacker) -> bool
		{
			if (sq == Square::none || attacker == Color::none)
				return false;

			const int s = static_cast<int>(sq);
			const int s_file = s % 8;
			const int s_rank = s / 8;

			auto in_bounds = [](int file, int rank) -> bool
				{
					return file >= 0 && file < 8 && rank >= 0 && rank < 8;
				};

			auto piece_at = [this](int file, int rank) -> PieceType
				{
					return bitboards.piece_at(make_square(file, rank));
				};

			if (attacker == Color::white)
			{
				if (in_bounds(s_file - 1, s_rank - 1) &&
					piece_at(s_file - 1, s_rank - 1) == PieceType::white_pawn)
					return true;
				if (in_bounds(s_file + 1, s_rank - 1) &&
					piece_at(s_file + 1, s_rank - 1) == PieceType::white_pawn)
					return true;
			}
			else
			{
				if (in_bounds(s_file - 1, s_rank + 1) &&
					piece_at(s_file - 1, s_rank + 1) == PieceType::black_pawn)
					return true;
				if (in_bounds(s_file + 1, s_rank + 1) &&
					piece_at(s_file + 1, s_rank + 1) == PieceType::black_pawn)
					return true;
			}

			const PieceType enemy_knight = (attacker == Color::white)
				? PieceType::white_knight
				: PieceType::black_knight;
			if (knight_attacks[s] & bitboards[enemy_knight].get())
				return true;

			const PieceType enemy_king = (attacker == Color::white)
				? PieceType::white_king
				: PieceType::black_king;
			if (king_attacks[s] & bitboards[enemy_king].get())
				return true;

			auto is_enemy_slider = [attacker](PieceType p, bool diagonal) -> bool
				{
					if (attacker == Color::white)
					{
						return diagonal ? (p == PieceType::white_bishop ||
							p == PieceType::white_queen)
							: (p == PieceType::white_rook ||
								p == PieceType::white_queen);
					}
					return diagonal
						? (p == PieceType::black_bishop || p == PieceType::black_queen)
						: (p == PieceType::black_rook || p == PieceType::black_queen);
				};

			auto ray_attacked = [&](int df, int dr, bool diagonal) -> bool
				{
					int f = s_file + df;
					int r = s_rank + dr;
					while (in_bounds(f, r))
					{
						const PieceType p = piece_at(f, r);
						if (p != PieceType::none)
							return is_enemy_slider(p, diagonal);
						f += df;
						r += dr;
					}
					return false;
				};

			if (ray_attacked(0, 1, false) || ray_attacked(0, -1, false) ||
				ray_attacked(1, 0, false) || ray_attacked(-1, 0, false))
				return true;

			if (ray_attacked(1, 1, true) || ray_attacked(1, -1, true) ||
				ray_attacked(-1, 1, true) || ray_attacked(-1, -1, true))
				return true;

			return false;
		};

	size_t write_idx = 0;
	for (size_t i = 0; i < original_count; ++i)
	{
		Move candidate = moves[i];

		const Color side_to_move = current_turn;
		const Color enemy_side =
			(side_to_move == Color::white) ? Color::black : Color::white;
		const PieceType enemy_king = (enemy_side == Color::white)
			? PieceType::white_king
			: PieceType::black_king;

		if (candidate.captured == enemy_king)
			continue;

		if (candidate.is_castle())
		{
			const Square through =
				(candidate.type == MoveType::kingside_castle)
				? ((side_to_move == Color::white) ? Square::F1 : Square::F8)
				: ((side_to_move == Color::white) ? Square::D1 : Square::D8);

			if (is_square_attacked_by(candidate.from, enemy_side) ||
				is_square_attacked_by(through, enemy_side))
				continue;
		}

		make_move(candidate);

		const Color side_just_moved =
			(current_turn == Color::white) ? Color::black : Color::white;
		const PieceType king_piece = (side_just_moved == Color::white)
			? PieceType::white_king
			: PieceType::black_king;
		const Square king_square = make_square(bitboards[king_piece].get());

		const bool in_check = (king_square == Square::none) ||
			is_square_attacked_by(king_square, current_turn);

		unmake_move(candidate);

		if (!in_check)
			moves[write_idx++] = candidate;
	}

	moves.count = write_idx;
	return moves;
}

MoveList& Board::get_pseudo_legal_moves()
{
	ZoneScoped;
	move_list.clear();
	generate_pawn_moves();
	generate_knight_moves();
	generate_bishop_moves();
	generate_king_moves();
	generate_queen_moves();
	generate_rook_moves();
	return move_list;
}

void Board::generate_pawn_moves()
{
	ZoneScoped;
	const auto empty_squares = get_empty_squares();
	const auto enemy_pieces = (current_turn == Color::white)
		? get_squares(Color::black)
		: get_squares(Color::white);

	auto pawns = (current_turn == Color::white)
		? bitboards[PieceType::white_pawn].get()
		: bitboards[PieceType::black_pawn].get();

	const auto dir =
		(current_turn == Color::white) ? Direction::north : Direction::south;
	const auto cap_left = (current_turn == Color::white) ? Direction::north_west
		: Direction::south_east;
	const auto cap_right = (current_turn == Color::white) ? Direction::north_east
		: Direction::south_west;
	const auto piece_type = (current_turn == Color::white)
		? PieceType::white_pawn
		: PieceType::black_pawn;
	const auto start_rank = (current_turn == Color::white) ? RANK_2 : RANK_7;
	const auto captured_pawn = (current_turn == Color::white)
		? PieceType::black_pawn
		: PieceType::white_pawn;

	while (pawns)
	{
		const auto from = std::countr_zero(pawns);
		const Bitboard from_bb{ 1ULL << from };

		const Bitboard left_attacks =
			(current_turn == Color::white)
			? ((from_bb & ~FILE_A).shift(cap_left, 1))
			: ((from_bb & ~FILE_H).shift(cap_left, 1));
		const Bitboard right_attacks =
			(current_turn == Color::white)
			? ((from_bb & ~FILE_H).shift(cap_right, 1))
			: ((from_bb & ~FILE_A).shift(cap_right, 1));

		Bitboard single_push_bb = from_bb.shift(dir, 1) & empty_squares;
		u64 s = single_push_bb.get();
		while (s)
		{
			const auto to = std::countr_zero(s);
			const bool is_promo =
				(current_turn == Color::white) ? (to >= 56) : (to < 8);
			if (is_promo)
			{
				for (const auto promo_type :
					{ MoveType::promotion_queen, MoveType::promotion_rook,
					 MoveType::promotion_bishop, MoveType::promotion_knight })
				{
					move_list.add(make_square(from), make_square(to), piece_type,
						promo_type, PieceType::none, castling_rights,
						Square::none, 0);
				}
			}
			else
			{
				move_list.add(make_square(from), make_square(to), piece_type,
					MoveType::normal, PieceType::none, castling_rights,
					Square::none, 0);
			}
			s &= s - 1;
		}

		if (from_bb.get() & start_rank)
		{
			Bitboard double_push_bb = from_bb.shift(dir, 2) & empty_squares &
				(single_push_bb.shift(dir, 1));
			u64 d = double_push_bb.get();
			while (d)
			{
				const auto to = std::countr_zero(d);
				// The en passant square is the square the pawn passed through
				const auto ep_sq = static_cast<Square>(
					(current_turn == Color::white) ? from + 8 : from - 8);
				move_list.add(make_square(from), make_square(to), piece_type,
					MoveType::double_pawn_push, PieceType::none,
					castling_rights, ep_sq, 0);
				d &= d - 1;
			}
		}

		const Bitboard attacks = (left_attacks | right_attacks) & enemy_pieces;
		u64 a = attacks.get();
		while (a)
		{
			const auto to = std::countr_zero(a);
			const auto to_sq = make_square(to);
			const auto captured_piece = bitboards.piece_at(to_sq);

			u8 new_castling = castling_rights;
			if (to == 7)
				new_castling &= ~castle_white_kingside; // H1
			else if (to == 0)
				new_castling &= ~castle_white_queenside; // A1
			else if (to == 63)
				new_castling &= ~castle_black_kingside; // H8
			else if (to == 56)
				new_castling &= ~castle_black_queenside; // A8

			const bool is_promo =
				(current_turn == Color::white) ? (to >= 56) : (to < 8);
			if (is_promo)
			{
				for (const auto promo_type : { MoveType::promotion_queen_capture,
											  MoveType::promotion_rook_capture,
											  MoveType::promotion_bishop_capture,
											  MoveType::promotion_knight_capture })
				{
					move_list.add(make_square(from), to_sq, piece_type, promo_type,
						captured_piece, new_castling, Square::none, 0);
				}
			}
			else
			{
				move_list.add(make_square(from), to_sq, piece_type, MoveType::capture,
					captured_piece, new_castling, Square::none, 0);
			}
			a &= a - 1;
		}

		if (en_passant_square != Square::none)
		{
			const u64 ep_bb = 1ULL << static_cast<int>(en_passant_square);
			const u64 ep_attacks = (left_attacks | right_attacks).get() & ep_bb;
			u64 ep = ep_attacks;
			while (ep)
			{
				const auto to = std::countr_zero(ep);
				move_list.add(make_square(from), make_square(to), piece_type,
					MoveType::en_passant, captured_pawn, castling_rights,
					Square::none, 0);
				ep &= ep - 1;
			}
		}

		pawns &= pawns - 1;
	}
}
void Board::generate_knight_moves()
{
	ZoneScoped;
	const auto friendly_squares =
		get_squares(current_turn == Color::white ? Color::white : Color::black);
	auto knights =
		bitboards[current_turn == Color::white ? PieceType::white_knight
		: PieceType::black_knight]
		.get();
	while (knights)
	{
		auto from_idx = std::countr_zero(knights);
		const auto from = make_square(from_idx);

		const u64 moves = knight_attacks[from_idx] & ~friendly_squares;

		move_list.add_moves(from, moves,
			current_turn == Color::white ? PieceType::white_knight
			: PieceType::black_knight,
			bitboards, castling_rights, Square::none,
			halfmove_clock);

		knights &= knights - 1;
	}
}
void Board::generate_bishop_moves()
{
	ZoneScoped;
	const auto piece_type = current_turn == Color::white
		? PieceType::white_bishop
		: PieceType::black_bishop;
	auto bishops = bitboards[piece_type].get();
	const u64 occupied = ~get_empty_squares();
	const u64 friendly =
		get_squares(current_turn == Color::white ? Color::white : Color::black);

	while (bishops)
	{
		const int from_idx = std::countr_zero(bishops);

		u64 attacks = 0;

		for (const auto& dir : { 1, 7 })
		{
			const u64 ray = ray_attacks[dir][from_idx];
			const u64 blockers = ray & occupied;
			if (blockers)
			{
				const int first_blocker = std::countr_zero(blockers);
				attacks |= ray ^ ray_attacks[dir][first_blocker];
			}
			else
				attacks |= ray;
		}

		for (const auto& dir : { 3, 5 })
		{
			const u64 ray = ray_attacks[dir][from_idx];
			const u64 blockers = ray & occupied;
			if (blockers)
			{
				const int first_blocker = 63 - std::countl_zero(blockers);
				attacks |= ray ^ ray_attacks[dir][first_blocker];
			}
			else
				attacks |= ray;
		}

		attacks &= ~friendly;
		move_list.add_moves(make_square(from_idx), attacks, piece_type, bitboards,
			castling_rights, Square::none, halfmove_clock);

		bishops &= bishops - 1;
	}
}
void Board::generate_rook_moves()
{
	ZoneScoped;
	const auto piece_type = current_turn == Color::white ? PieceType::white_rook
		: PieceType::black_rook;
	auto rooks = bitboards[piece_type].get();
	const u64 occupied = ~get_empty_squares();
	const u64 friendly =
		get_squares(current_turn == Color::white ? Color::white : Color::black);

	while (rooks)
	{
		const int from_idx = std::countr_zero(rooks);

		u8 new_castling = castling_rights;
		if (piece_type == PieceType::white_rook)
		{
			if (from_idx == 7)
				new_castling &= ~castle_white_kingside;
			else if (from_idx == 0)
				new_castling &= ~castle_white_queenside;
		}
		else
		{
			if (from_idx == 63)
				new_castling &= ~castle_black_kingside;
			else if (from_idx == 56)
				new_castling &= ~castle_black_queenside;
		}

		u64 attacks = 0;

		for (int dir : {0, 2})
		{
			const u64 ray = ray_attacks[dir][from_idx];
			const u64 blockers = ray & occupied;
			if (blockers)
			{
				const int first_blocker = std::countr_zero(blockers);
				attacks |= ray ^ ray_attacks[dir][first_blocker];
			}
			else
				attacks |= ray;
		}

		for (int dir : {4, 6})
		{
			const u64 ray = ray_attacks[dir][from_idx];
			const u64 blockers = ray & occupied;
			if (blockers)
			{
				const int first_blocker = 63 - std::countl_zero(blockers);
				attacks |= ray ^ ray_attacks[dir][first_blocker];
			}
			else
				attacks |= ray;
		}

		attacks &= ~friendly;
		move_list.add_moves(make_square(from_idx), attacks, piece_type, bitboards,
			new_castling, Square::none, halfmove_clock);

		rooks &= rooks - 1;
	}
}
void Board::generate_queen_moves()
{
	ZoneScoped;
	const auto piece_type = current_turn == Color::white ? PieceType::white_queen
		: PieceType::black_queen;
	auto queens = bitboards[piece_type].get();
	const u64 occupied = ~get_empty_squares();
	const u64 friendly =
		get_squares(current_turn == Color::white ? Color::white : Color::black);

	while (queens)
	{
		const int from_idx = std::countr_zero(queens);

		u64 attacks = 0;

		for (int dir : {0, 1, 2, 7})
		{
			const u64 ray = ray_attacks[dir][from_idx];
			const u64 blockers = ray & occupied;
			if (blockers)
			{
				const int first_blocker = std::countr_zero(blockers);
				attacks |= ray ^ ray_attacks[dir][first_blocker];
			}
			else
				attacks |= ray;
		}

		for (int dir : {3, 4, 5, 6})
		{
			const u64 ray = ray_attacks[dir][from_idx];
			const u64 blockers = ray & occupied;
			if (blockers)
			{
				const int first_blocker = 63 - std::countl_zero(blockers);
				attacks |= ray ^ ray_attacks[dir][first_blocker];
			}
			else
				attacks |= ray;
		}

		attacks &= ~friendly;
		move_list.add_moves(make_square(from_idx), attacks, piece_type, bitboards,
			castling_rights, Square::none, halfmove_clock);
		queens &= queens - 1;
	}
}
void Board::generate_king_moves()
{
	ZoneScoped;
	const auto friendly_squares =
		get_squares(current_turn == Color::white ? Color::white : Color::black);
	const auto piece_type = current_turn == Color::white ? PieceType::white_king
		: PieceType::black_king;
	auto king = bitboards[piece_type].get();
	const auto from = std::countr_zero(king);

	u8 king_castling = castling_rights;
	if (current_turn == Color::white)
		king_castling &= ~(castle_white_kingside | castle_white_queenside);
	else
		king_castling &= ~(castle_black_kingside | castle_black_queenside);

	auto moves = king_attacks[from] & ~friendly_squares;
	while (moves)
	{
		const auto move_idx = std::countr_zero(moves);
		const auto to_sq = make_square(move_idx);
		const auto captured = bitboards.piece_at(to_sq);
		const auto type =
			(captured != PieceType::none) ? MoveType::capture : MoveType::normal;

		u8 move_castling = king_castling;
		if (move_idx == 7)
			move_castling &= ~castle_white_kingside; // H1
		else if (move_idx == 0)
			move_castling &= ~castle_white_queenside; // A1
		else if (move_idx == 63)
			move_castling &= ~castle_black_kingside; // H8
		else if (move_idx == 56)
			move_castling &= ~castle_black_queenside; // A8

		const u8 new_halfmove =
			(captured != PieceType::none) ? 0 : static_cast<u8>(halfmove_clock + 1);

		move_list.add(make_square(from), to_sq, piece_type, type, captured,
			move_castling, Square::none, new_halfmove);
		moves &= moves - 1;
	}

	// --- castling ---
	const u64 occupied = ~get_empty_squares();

	if (current_turn == Color::white && from == static_cast<int>(Square::E1))
	{
		if ((castling_rights & castle_white_kingside) && !(occupied & 0x60ULL) &&
			(bitboards[PieceType::white_rook].get() & 0x80ULL))
		{
			move_list.add(Square::E1, Square::G1, piece_type,
				MoveType::kingside_castle, PieceType::none, king_castling,
				Square::none, static_cast<u8>(halfmove_clock + 1));
		}
		if ((castling_rights & castle_white_queenside) && !(occupied & 0xEULL) &&
			(bitboards[PieceType::white_rook].get() & 0x1ULL))
		{
			move_list.add(Square::E1, Square::C1, piece_type,
				MoveType::queenside_castle, PieceType::none, king_castling,
				Square::none, static_cast<u8>(halfmove_clock + 1));
		}
	}
	else if (current_turn == Color::black &&
		from == static_cast<int>(Square::E8))
	{
		if ((castling_rights & castle_black_kingside) &&
			!(occupied & 0x6000000000000000ULL) &&
			(bitboards[PieceType::black_rook].get() & 0x8000000000000000ULL))
		{
			move_list.add(Square::E8, Square::G8, piece_type,
				MoveType::kingside_castle, PieceType::none, king_castling,
				Square::none, static_cast<u8>(halfmove_clock + 1));
		}
		if ((castling_rights & castle_black_queenside) &&
			!(occupied & 0xE00000000000000ULL) &&
			(bitboards[PieceType::black_rook].get() & 0x100000000000000ULL))
		{
			move_list.add(Square::E8, Square::C8, piece_type,
				MoveType::queenside_castle, PieceType::none, king_castling,
				Square::none, static_cast<u8>(halfmove_clock + 1));
		}
	}
}

void Board::print() const
{
	constexpr char pieces[] = "PNBRQKpnbrqk";

	for (int rank = 7; rank >= 0; rank--)
	{
		std::cout << rank + 1 << "  ";
		for (int file = 0; file < 8; file++)
		{
			const int square = rank * 8 + file;
			char found = '.';

			for (int i = 0; i < 12; i++)
			{
				if (bitboards[i] & (1ULL << square))
				{
					found = pieces[i];
					break;
				}
			}
			std::cout << found << ' ';
		}
		std::cout << '\n';
	}
	std::cout << "   A B C D E F G H\n\n";
}

u64 Board::get_squares(Color color) const { return bitboards.occupied(color); }

constexpr void Board::init_knight_attacks()
{
	for (int s = 0; s < 64; ++s)
	{
		u64 b = 1ULL << s;
		u64 attacks = 0;

		attacks |= (b << 17) & ~FILE_A;
		attacks |= (b << 15) & ~FILE_H;
		attacks |= (b >> 17) & ~FILE_H;
		attacks |= (b >> 15) & ~FILE_A;

		attacks |= (b << 10) & ~(FILE_A | FILE_B);
		attacks |= (b << 6) & ~(FILE_G | FILE_H);
		attacks |= (b >> 10) & ~(FILE_G | FILE_H);
		attacks |= (b >> 6) & ~(FILE_A | FILE_B);

		knight_attacks[s] = attacks;
	}
}

constexpr void Board::init_king_attacks()
{
	for (int s = 0; s < 64; ++s)
	{
		u64 b = 1ULL << s;
		u64 attacks = 0;

		attacks |= (b << 8);
		attacks |= (b >> 8);

		attacks |= (b << 1) & ~FILE_A;
		attacks |= (b >> 1) & ~FILE_H;

		attacks |= (b << 9) & ~FILE_A;
		attacks |= (b << 7) & ~FILE_H;
		attacks |= (b >> 9) & ~FILE_H;
		attacks |= (b >> 7) & ~FILE_A;

		king_attacks[s] = attacks;
	}
}

constexpr void Board::init_ray_attacks()
{
	for (int s = 0; s < 64; ++s)
	{
		int rank = s / 8;
		int file = s % 8;

		u64 ray = 0;
		for (int r = rank + 1; r < 8; ++r)
			ray |= 1ULL << (r * 8 + file);
		ray_attacks[0][s] = ray;

		ray = 0;
		for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f)
			ray |= 1ULL << (r * 8 + f);
		ray_attacks[1][s] = ray;

		ray = 0;
		for (int f = file + 1; f < 8; f++)
			ray |= 1ULL << (rank * 8 + f);
		ray_attacks[2][s] = ray;

		ray = 0;
		for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f)
			ray |= 1ULL << (r * 8 + f);
		ray_attacks[3][s] = ray;

		ray = 0;
		for (int r = rank - 1; r >= 0; --r)
			ray |= 1ULL << (r * 8 + file);
		ray_attacks[4][s] = ray;

		ray = 0;
		for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f)
			ray |= 1ULL << (r * 8 + f);
		ray_attacks[5][s] = ray;

		ray = 0;
		for (int f = file - 1; f >= 0; --f)
			ray |= 1ULL << (rank * 8 + f);
		ray_attacks[6][s] = ray;

		ray = 0;
		for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f)
			ray |= 1ULL << (r * 8 + f);
		ray_attacks[7][s] = ray;
	}
}

bool is_number(char c) { return std::isdigit(static_cast<unsigned char>(c)); }

void Board::read_fen(const std::string& fen)
{
	ZoneScoped;
	assert(!fen.empty());
	clear();

	auto split_fen = [](std::string_view fen)
		{
			std::array<std::string_view, 6> fields{};

			size_t start = 0;
			size_t end;
			int i = 0;

			while ((end = fen.find(' ', start)) != std::string_view::npos && i < 5)
			{
				fields[i++] = fen.substr(start, end - start);
				start = end + 1;
			}

			fields[i] = fen.substr(start);

			return fields;
		};

	auto parse_square = [](const std::string_view sq) -> Square
		{
			if (sq.size() != 2)
				return Square::none;

			const char file_char =
				static_cast<char>(std::tolower(static_cast<unsigned char>(sq[0])));
			const char rank_char = sq[1];
			if (file_char < 'a' || file_char > 'h' || rank_char < '1' ||
				rank_char > '8')
				return Square::none;

			const int file_idx = file_char - 'a';
			const int rank_idx = rank_char - '1';
			return make_square(file_idx, rank_idx);
		};

	const auto fields = split_fen(fen);

	static const std::unordered_map<char, PieceType> charToPiece = {
		{'P', PieceType::white_pawn},   {'N', PieceType::white_knight},
		{'B', PieceType::white_bishop}, {'R', PieceType::white_rook},
		{'Q', PieceType::white_queen},  {'K', PieceType::white_king},
		{'p', PieceType::black_pawn},   {'n', PieceType::black_knight},
		{'b', PieceType::black_bishop}, {'r', PieceType::black_rook},
		{'q', PieceType::black_queen},  {'k', PieceType::black_king},
	};

	int ctr = 0;
	for (const char c : fields[0])
	{
		if (c == '/')
			continue;

		if (is_number(c))
		{
			ctr += c - '0';
			continue;
		}

		if (auto it = charToPiece.find(c); it != charToPiece.end())
		{
			const int rank = 7 - (ctr / 8);
			const int file = ctr % 8;
			const int square = rank * 8 + file;
			bitboards[it->second] |= (1ULL << square);
			ctr++;
		}
	}

	if (fields[1] == "w")
		current_turn = Color::white;
	else if (fields[1] == "b")
		current_turn = Color::black;

	if (fields[2] != "-")
	{
		for (const char& c : fields[2])
		{
			if (c == 'K')
				castling_rights |= castle_white_kingside;
			else if (c == 'Q')
				castling_rights |= castle_white_queenside;
			else if (c == 'k')
				castling_rights |= castle_black_kingside;
			else if (c == 'q')
				castling_rights |= castle_black_queenside;
		}
	}

	en_passant_square =
		(fields[3] == "-") ? Square::none : parse_square(fields[3]);

	if (!fields[4].empty())
		halfmove_clock = static_cast<u8>(std::stoi(std::string(fields[4])));

	if (!fields[5].empty())
		fullmove_number = std::stoi(std::string(fields[5]));
}

void Board::make_move(Move& move)
{
	ZoneScoped;
	move.prev_castling_rights = castling_rights;
	move.prev_enpassant_sq = en_passant_square;
	move.prev_halfmove_clock = halfmove_clock;
	move.prev_fullmove_number = fullmove_number;
	move.prev_turn = current_turn;

	make_move(static_cast<const Move&>(move));
}

void Board::make_move(const Move& move)
{
	ZoneScoped;
	previous_move = move;
	previous_move.prev_castling_rights = castling_rights;
	previous_move.prev_enpassant_sq = en_passant_square;
	previous_move.prev_halfmove_clock = halfmove_clock;
	previous_move.prev_fullmove_number = fullmove_number;
	previous_move.prev_turn = current_turn;

	const u64 from_bb = 1ULL << static_cast<int>(move.from);
	const u64 to_bb = 1ULL << static_cast<int>(move.to);

	bitboards[move.piece] &= ~from_bb;

	if (move.type == MoveType::kingside_castle)
	{
		bitboards[move.piece] |= to_bb;
		if (current_turn == Color::white)
		{
			bitboards[PieceType::white_rook] &=
				~(1ULL << static_cast<int>(Square::H1));
			bitboards[PieceType::white_rook] |=
				(1ULL << static_cast<int>(Square::F1));
		}
		else
		{
			bitboards[PieceType::black_rook] &=
				~(1ULL << static_cast<int>(Square::H8));
			bitboards[PieceType::black_rook] |=
				(1ULL << static_cast<int>(Square::F8));
		}
	}
	else if (move.type == MoveType::queenside_castle)
	{
		bitboards[move.piece] |= to_bb;
		if (current_turn == Color::white)
		{
			bitboards[PieceType::white_rook] &=
				~(1ULL << static_cast<int>(Square::A1));
			bitboards[PieceType::white_rook] |=
				(1ULL << static_cast<int>(Square::D1));
		}
		else
		{
			bitboards[PieceType::black_rook] &=
				~(1ULL << static_cast<int>(Square::A8));
			bitboards[PieceType::black_rook] |=
				(1ULL << static_cast<int>(Square::D8));
		}
	}
	else if (move.type == MoveType::en_passant)
	{
		bitboards[move.piece] |= to_bb;
		const int captured_sq = (current_turn == Color::white)
			? static_cast<int>(move.to) - 8
			: static_cast<int>(move.to) + 8;
		bitboards[move.captured] &= ~(1ULL << captured_sq);
	}
	else if (move.is_promotion())
	{
		if (move.captured != PieceType::none)
			bitboards[move.captured] &= ~to_bb;

		const bool is_white = (current_turn == Color::white);
		PieceType promo_piece;
		switch (move.type)
		{
		case MoveType::promotion_queen:
		case MoveType::promotion_queen_capture:
			promo_piece = is_white ? PieceType::white_queen : PieceType::black_queen;
			break;
		case MoveType::promotion_rook:
		case MoveType::promotion_rook_capture:
			promo_piece = is_white ? PieceType::white_rook : PieceType::black_rook;
			break;
		case MoveType::promotion_bishop:
		case MoveType::promotion_bishop_capture:
			promo_piece =
				is_white ? PieceType::white_bishop : PieceType::black_bishop;
			break;
		default:
			promo_piece =
				is_white ? PieceType::white_knight : PieceType::black_knight;
			break;
		}
		bitboards[promo_piece] |= to_bb;
	}
	else
	{
		if (move.captured != PieceType::none)
			bitboards[move.captured] &= ~to_bb;
		bitboards[move.piece] |= to_bb;
	}

	castling_rights = move.castling_rights;
	en_passant_square = move.enpassant_sq;
	halfmove_clock = move.halfmove_clock;
	if (current_turn == Color::black)
		fullmove_number++;
	current_turn = (current_turn == Color::white) ? Color::black : Color::white;
}
