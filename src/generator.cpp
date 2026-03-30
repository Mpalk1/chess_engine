#include "generator.h"

#include <bit>

#include "bitboard.h"
#include "position.h"

const std::array<u64, 64> Generator::knight_attacks = Generator::init_knight_attacks();
const std::array<u64, 64> Generator::king_attacks = Generator::init_king_attacks();
const std::array<std::array<u64, 64>, 8> Generator::ray_attacks = Generator::init_ray_attacks();

MoveList& Generator::get_moves(Position& position, MoveList& out_moves)
{
	out_moves.clear();

	PositionInfo info{};
	info.turn = position.current_turn;
	info.empty = position.get_empty_squares();
	info.occupied = ~info.empty;
	info.friendly = position.get_squares(info.turn);
	info.enemy = info.occupied & ~info.friendly;

	generate_pawn_moves(position, info, out_moves);
	generate_knight_moves(position, info, out_moves);
	generate_bishop_moves(position, info, out_moves);
	generate_rook_moves(position, info, out_moves);
	generate_queen_moves(position, info, out_moves);
	generate_king_moves(position, info, out_moves);

	filter_legal_moves(position, out_moves);

	return out_moves;
}

bool Generator::is_square_attacked_by(const Position& position, Square sq, Color attacker)
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

	auto piece_at = [&position](int file, int rank) -> PieceType
		{
			return position.piece_at(make_square(file, rank));
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
	if (Generator::knight_attacks[s] & position.bitboards[enemy_knight].get())
		return true;

	const PieceType enemy_king = (attacker == Color::white)
		? PieceType::white_king
		: PieceType::black_king;
	if (Generator::king_attacks[s] & position.bitboards[enemy_king].get())
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
}

void Generator::filter_legal_moves(Position& position, MoveList& moves)
{
	const size_t original_count = moves.count;

	size_t write_idx = 0;
	for (size_t i = 0; i < original_count; ++i)
	{
		Move candidate = moves[i];

		const Color side_to_move = position.current_turn;
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

			if (is_square_attacked_by(position, candidate.from, enemy_side) ||
				is_square_attacked_by(position, through, enemy_side))
				continue;
		}

		position.make_move(candidate);

		const Color side_just_moved =
			(position.current_turn == Color::white) ? Color::black : Color::white;
		const PieceType king_piece = (side_just_moved == Color::white)
			? PieceType::white_king
			: PieceType::black_king;
		const Square king_square = make_square(position.bitboards[king_piece].get());

		const bool in_check = (king_square == Square::none) ||
			is_square_attacked_by(position, king_square, position.current_turn);

		position.unmake_move(candidate);

		if (!in_check)
			moves[write_idx++] = candidate;
	}

	moves.count = write_idx;
}

void Generator::generate_pawn_moves(Position& position, const PositionInfo& info,
	MoveList& out_moves)
{
	const u64 empty_squares = info.empty;
	const u64 enemy_pieces = info.enemy;

	auto pawns = (info.turn == Color::white)
		? position.bitboards[PieceType::white_pawn].get()
		: position.bitboards[PieceType::black_pawn].get();

	const auto dir =
		(info.turn == Color::white) ? Direction::north : Direction::south;
	const auto cap_left = (info.turn == Color::white) ? Direction::north_west
		: Direction::south_east;
	const auto cap_right = (info.turn == Color::white) ? Direction::north_east
		: Direction::south_west;
	const auto piece_type = (info.turn == Color::white)
		? PieceType::white_pawn
		: PieceType::black_pawn;
	const auto start_rank = (info.turn == Color::white) ? RANK_2 : RANK_7;
	const auto captured_pawn = (info.turn == Color::white)
		? PieceType::black_pawn
		: PieceType::white_pawn;

	while (pawns)
	{
		const auto from = std::countr_zero(pawns);
		const Bitboard from_bb{ 1ULL << from };

		const Bitboard left_attacks =
			(info.turn == Color::white)
			? ((from_bb & ~FILE_A).shift(cap_left, 1))
			: ((from_bb & ~FILE_H).shift(cap_left, 1));
		const Bitboard right_attacks =
			(info.turn == Color::white)
			? ((from_bb & ~FILE_H).shift(cap_right, 1))
			: ((from_bb & ~FILE_A).shift(cap_right, 1));

		Bitboard single_push_bb = from_bb.shift(dir, 1) & empty_squares;
		u64 s = single_push_bb.get();
		while (s)
		{
			const auto to = std::countr_zero(s);
			const bool is_promo =
				(info.turn == Color::white) ? (to >= 56) : (to < 8);
			if (is_promo)
			{
				for (const auto promo_type :
					{ MoveType::promotion_queen, MoveType::promotion_rook,
					 MoveType::promotion_bishop, MoveType::promotion_knight })
				{
					out_moves.add(make_square(from), make_square(to), piece_type,
						promo_type, PieceType::none, position.castling_rights,
						Square::none, 0);
				}
			}
			else
			{
				out_moves.add(make_square(from), make_square(to), piece_type,
					MoveType::normal, PieceType::none, position.castling_rights,
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
				const auto ep_sq = static_cast<Square>(
					(info.turn == Color::white) ? from + 8 : from - 8);
				out_moves.add(make_square(from), make_square(to), piece_type,
					MoveType::double_pawn_push, PieceType::none,
					position.castling_rights, ep_sq, 0);
				d &= d - 1;
			}
		}

		const Bitboard attacks = (left_attacks | right_attacks) & enemy_pieces;
		u64 a = attacks.get();
		while (a)
		{
			const auto to = std::countr_zero(a);
			const auto to_sq = make_square(to);
			const auto captured_piece = position.piece_at(to_sq);

			u8 new_castling = position.castling_rights;
			if (to == 7)
				new_castling &= ~Position::castle_white_kingside;
			else if (to == 0)
				new_castling &= ~Position::castle_white_queenside;
			else if (to == 63)
				new_castling &= ~Position::castle_black_kingside;
			else if (to == 56)
				new_castling &= ~Position::castle_black_queenside;

			const bool is_promo =
				(info.turn == Color::white) ? (to >= 56) : (to < 8);
			if (is_promo)
			{
				for (const auto promo_type : { MoveType::promotion_queen_capture,
											  MoveType::promotion_rook_capture,
											  MoveType::promotion_bishop_capture,
											  MoveType::promotion_knight_capture })
				{
					out_moves.add(make_square(from), to_sq, piece_type, promo_type,
						captured_piece, new_castling, Square::none, 0);
				}
			}
			else
			{
				out_moves.add(make_square(from), to_sq, piece_type, MoveType::capture,
					captured_piece, new_castling, Square::none, 0);
			}
			a &= a - 1;
		}

		if (position.en_passant_square != Square::none)
		{
			const u64 ep_bb = 1ULL << static_cast<int>(position.en_passant_square);
			const u64 ep_attacks = (left_attacks | right_attacks).get() & ep_bb;
			u64 ep = ep_attacks;
			while (ep)
			{
				const auto to = std::countr_zero(ep);
				out_moves.add(make_square(from), make_square(to), piece_type,
					MoveType::en_passant, captured_pawn, position.castling_rights,
					Square::none, 0);
				ep &= ep - 1;
			}
		}

		pawns &= pawns - 1;
	}
}

void Generator::generate_knight_moves(Position& position, const PositionInfo& info,
	MoveList& out_moves)
{
	auto knights =
		position.bitboards[info.turn == Color::white ? PieceType::white_knight
		: PieceType::black_knight]
		.get();
	while (knights)
	{
		auto from_idx = std::countr_zero(knights);
		const auto from = make_square(from_idx);

		const u64 moves = knight_attacks[from_idx] & ~info.friendly;

		out_moves.add_moves(from, moves,
			info.turn == Color::white ? PieceType::white_knight
			: PieceType::black_knight,
			position, position.castling_rights, Square::none,
			position.halfmove_clock);

		knights &= knights - 1;
	}
}

void Generator::generate_bishop_moves(Position& position, const PositionInfo& info,
	MoveList& out_moves)
{
	const auto piece_type = info.turn == Color::white
		? PieceType::white_bishop
		: PieceType::black_bishop;
	auto bishops = position.bitboards[piece_type].get();
	const u64 occupied = info.occupied;

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

		attacks &= ~info.friendly;
		out_moves.add_moves(make_square(from_idx), attacks, piece_type, position,
			position.castling_rights, Square::none, position.halfmove_clock);

		bishops &= bishops - 1;
	}
}

void Generator::generate_rook_moves(Position& position, const PositionInfo& info,
	MoveList& out_moves)
{
	const auto piece_type = info.turn == Color::white ? PieceType::white_rook
		: PieceType::black_rook;
	auto rooks = position.bitboards[piece_type].get();
	const u64 occupied = info.occupied;

	while (rooks)
	{
		const int from_idx = std::countr_zero(rooks);

		u8 new_castling = position.castling_rights;
		if (piece_type == PieceType::white_rook)
		{
			if (from_idx == 7)
				new_castling &= ~Position::castle_white_kingside;
			else if (from_idx == 0)
				new_castling &= ~Position::castle_white_queenside;
		}
		else
		{
			if (from_idx == 63)
				new_castling &= ~Position::castle_black_kingside;
			else if (from_idx == 56)
				new_castling &= ~Position::castle_black_queenside;
		}

		u64 attacks = 0;

		for (int dir : { 0, 2 })
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

		for (int dir : { 4, 6 })
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

		attacks &= ~info.friendly;
		out_moves.add_moves(make_square(from_idx), attacks, piece_type, position,
			new_castling, Square::none, position.halfmove_clock);

		rooks &= rooks - 1;
	}
}

void Generator::generate_queen_moves(Position& position, const PositionInfo& info,
	MoveList& out_moves)
{
	const auto piece_type = info.turn == Color::white ? PieceType::white_queen
		: PieceType::black_queen;
	auto queens = position.bitboards[piece_type].get();
	const u64 occupied = info.occupied;

	while (queens)
	{
		const int from_idx = std::countr_zero(queens);

		u64 attacks = 0;

		for (int dir : { 0, 1, 2, 7 })
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

		for (int dir : { 3, 4, 5, 6 })
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

		attacks &= ~info.friendly;
		out_moves.add_moves(make_square(from_idx), attacks, piece_type, position,
			position.castling_rights, Square::none, position.halfmove_clock);
		queens &= queens - 1;
	}
}

void Generator::generate_king_moves(Position& position, const PositionInfo& info,
	MoveList& out_moves)
{
	const auto piece_type = info.turn == Color::white ? PieceType::white_king
		: PieceType::black_king;
	auto king = position.bitboards[piece_type].get();
	const auto from = std::countr_zero(king);

	u8 king_castling = position.castling_rights;
	if (info.turn == Color::white)
		king_castling &= ~(Position::castle_white_kingside | Position::castle_white_queenside);
	else
		king_castling &= ~(Position::castle_black_kingside | Position::castle_black_queenside);

	auto moves = king_attacks[from] & ~info.friendly;
	while (moves)
	{
		const auto move_idx = std::countr_zero(moves);
		const auto to_sq = make_square(move_idx);
		const auto captured = position.piece_at(to_sq);
		const auto type =
			(captured != PieceType::none) ? MoveType::capture : MoveType::normal;

		u8 move_castling = king_castling;
		if (move_idx == 7)
			move_castling &= ~Position::castle_white_kingside;
		else if (move_idx == 0)
			move_castling &= ~Position::castle_white_queenside;
		else if (move_idx == 63)
			move_castling &= ~Position::castle_black_kingside;
		else if (move_idx == 56)
			move_castling &= ~Position::castle_black_queenside;

		const u8 new_halfmove =
			(captured != PieceType::none) ? 0 : static_cast<u8>(position.halfmove_clock + 1);

		out_moves.add(make_square(from), to_sq, piece_type, type, captured,
			move_castling, Square::none, new_halfmove);
		moves &= moves - 1;
	}

	const u64 occupied = info.occupied;

	if (info.turn == Color::white && from == static_cast<int>(Square::E1))
	{
		if ((position.castling_rights & Position::castle_white_kingside) && !(occupied & 0x60ULL) &&
			(position.bitboards[PieceType::white_rook].get() & 0x80ULL))
		{
			out_moves.add(Square::E1, Square::G1, piece_type,
				MoveType::kingside_castle, PieceType::none, king_castling,
				Square::none, static_cast<u8>(position.halfmove_clock + 1));
		}
		if ((position.castling_rights & Position::castle_white_queenside) && !(occupied & 0xEULL) &&
			(position.bitboards[PieceType::white_rook].get() & 0x1ULL))
		{
			out_moves.add(Square::E1, Square::C1, piece_type,
				MoveType::queenside_castle, PieceType::none, king_castling,
				Square::none, static_cast<u8>(position.halfmove_clock + 1));
		}
	}
	else if (info.turn == Color::black &&
		from == static_cast<int>(Square::E8))
	{
		if ((position.castling_rights & Position::castle_black_kingside) &&
			!(occupied & 0x6000000000000000ULL) &&
			(position.bitboards[PieceType::black_rook].get() & 0x8000000000000000ULL))
		{
			out_moves.add(Square::E8, Square::G8, piece_type,
				MoveType::kingside_castle, PieceType::none, king_castling,
				Square::none, static_cast<u8>(position.halfmove_clock + 1));
		}
		if ((position.castling_rights & Position::castle_black_queenside) &&
			!(occupied & 0xE00000000000000ULL) &&
			(position.bitboards[PieceType::black_rook].get() & 0x100000000000000ULL))
		{
			out_moves.add(Square::E8, Square::C8, piece_type,
				MoveType::queenside_castle, PieceType::none, king_castling,
				Square::none, static_cast<u8>(position.halfmove_clock + 1));
		}
	}
}

std::array<u64, 64> Generator::init_knight_attacks()
{
	std::array<u64, 64> knights{};
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

		knights[s] = attacks;
	}
	return knights;
}

std::array<u64, 64> Generator::init_king_attacks()
{
	std::array<u64, 64> kings{};
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

		kings[s] = attacks;
	}
	return kings;
}

std::array<std::array<u64, 64>, 8> Generator::init_ray_attacks()
{
	std::array<std::array<u64, 64>, 8> rays{};
	for (int s = 0; s < 64; ++s)
	{
		int rank = s / 8;
		int file = s % 8;

		u64 ray = 0;
		for (int r = rank + 1; r < 8; ++r)
			ray |= 1ULL << (r * 8 + file);
		rays[0][s] = ray;

		ray = 0;
		for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f)
			ray |= 1ULL << (r * 8 + f);
		rays[1][s] = ray;

		ray = 0;
		for (int f = file + 1; f < 8; f++)
			ray |= 1ULL << (rank * 8 + f);
		rays[2][s] = ray;

		ray = 0;
		for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f)
			ray |= 1ULL << (r * 8 + f);
		rays[3][s] = ray;

		ray = 0;
		for (int r = rank - 1; r >= 0; --r)
			ray |= 1ULL << (r * 8 + file);
		rays[4][s] = ray;

		ray = 0;
		for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f)
			ray |= 1ULL << (r * 8 + f);
		rays[5][s] = ray;

		ray = 0;
		for (int f = file - 1; f >= 0; --f)
			ray |= 1ULL << (rank * 8 + f);
		rays[6][s] = ray;

		ray = 0;
		for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f)
			ray |= 1ULL << (r * 8 + f);
		rays[7][s] = ray;
	}
	return rays;
}

