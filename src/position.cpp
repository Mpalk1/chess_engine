#include "position.h"
#include "bitboard.h"
#include "generator.h"
#include "move_list.h"
#include "types.h"
#include "zobrist.h"
#include <cassert>
#include <cctype>
#include <iostream>
#include <string_view>
#include <unordered_map>

namespace
{
PieceType promoted_piece_for_move(MoveType type, Color side)
{
	const bool is_white = (side == Color::white);
	switch (type)
	{
	case MoveType::promotion_queen:
	case MoveType::promotion_queen_capture:
		return is_white ? PieceType::white_queen : PieceType::black_queen;
	case MoveType::promotion_rook:
	case MoveType::promotion_rook_capture:
		return is_white ? PieceType::white_rook : PieceType::black_rook;
	case MoveType::promotion_bishop:
	case MoveType::promotion_bishop_capture:
		return is_white ? PieceType::white_bishop : PieceType::black_bishop;
	default:
		return is_white ? PieceType::white_knight : PieceType::black_knight;
	}
}

void clear_square(Position& pos, Square sq, PieceType piece)
{
	if (sq == Square::none || piece == PieceType::none)
		return;

	const int idx = static_cast<int>(sq);
	pos.mailbox[idx] = PieceType::none;
	pos.bitboards[piece] &= ~(1ULL << idx);
}

void set_square(Position& pos, Square sq, PieceType piece)
{
	if (sq == Square::none || piece == PieceType::none)
		return;

	const int idx = static_cast<int>(sq);
	pos.mailbox[idx] = piece;
	pos.bitboards[piece] |= (1ULL << idx);
}
}

Position::Position() = default;

void Position::clear()
{
	bitboards.clear();
	mailbox.fill(PieceType::none);
	move_list.clear();
	current_turn = Color::white;
	castling_rights = 0;
	en_passant_square = Square::none;
	halfmove_clock = 0;
	fullmove_number = 1;
	zobrist_key = 0;
}

void Position::apply_move(const Move& move, MoveState& state)
{
	state.castling_rights = castling_rights;
	state.enpassant_sq = en_passant_square;
	state.halfmove_clock = halfmove_clock;
	state.fullmove_number = fullmove_number;
	state.turn = current_turn;

	if (move.piece == PieceType::none)
		return;

	if (move.type == MoveType::kingside_castle)
	{
		clear_square(*this, move.to, move.piece);
		set_square(*this, move.from, move.piece);

		if (state.turn == Color::white)
		{
			clear_square(*this, Square::F1, PieceType::white_rook);
			set_square(*this, Square::H1, PieceType::white_rook);
		}
		else
		{
			clear_square(*this, Square::F8, PieceType::black_rook);
			set_square(*this, Square::H8, PieceType::black_rook);
		}
	}
	else if (move.type == MoveType::queenside_castle)
	{
		clear_square(*this, move.to, move.piece);
		set_square(*this, move.from, move.piece);

		if (state.turn == Color::white)
		{
			clear_square(*this, Square::D1, PieceType::white_rook);
			set_square(*this, Square::A1, PieceType::white_rook);
		}
		else
		{
			clear_square(*this, Square::D8, PieceType::black_rook);
			set_square(*this, Square::A8, PieceType::black_rook);
		}
	}
	else if (move.type == MoveType::en_passant)
	{
		clear_square(*this, move.to, move.piece);
		set_square(*this, move.from, move.piece);

		const int captured_sq = (state.turn == Color::white)
			? static_cast<int>(move.to) - 8
			: static_cast<int>(move.to) + 8;
		set_square(*this, make_square(captured_sq), move.captured);
	}
	else if (move.is_promotion())
	{
		const PieceType promo_piece = promoted_piece_for_move(move.type, current_turn);

		clear_square(*this, move.to, promo_piece);
		set_square(*this, move.from, move.piece);

		if (move.captured != PieceType::none)
			set_square(*this, move.to, move.captured);
	}
	else
	{
		clear_square(*this, move.to, move.piece);
		set_square(*this, move.from, move.piece);

		if (move.captured != PieceType::none)
			set_square(*this, move.to, move.captured);
	}
}

void Position::undo_move(const Move& move, const MoveState& state)
{
	if (move.piece == PieceType::none)
		return;

	current_turn = state.turn;
	castling_rights = state.castling_rights;
	en_passant_square = state.enpassant_sq;
	halfmove_clock = state.halfmove_clock;
	fullmove_number = state.fullmove_number;

	if (move.type == MoveType::kingside_castle)
	{
		clear_square(*this, move.to, move.piece);
		set_square(*this, move.from, move.piece);

		if (state.turn == Color::white)
		{
			clear_square(*this, Square::F1, PieceType::white_rook);
			set_square(*this, Square::H1, PieceType::white_rook);
		}
		else
		{
			clear_square(*this, Square::F8, PieceType::black_rook);
			set_square(*this, Square::H8, PieceType::black_rook);
		}
	}
	else if (move.type == MoveType::queenside_castle)
	{
		clear_square(*this, move.to, move.piece);
		set_square(*this, move.from, move.piece);

		if (state.turn == Color::white)
		{
			clear_square(*this, Square::D1, PieceType::white_rook);
			set_square(*this, Square::A1, PieceType::white_rook);
		}
		else
		{
			clear_square(*this, Square::D8, PieceType::black_rook);
			set_square(*this, Square::A8, PieceType::black_rook);
		}
	}
	else if (move.type == MoveType::en_passant)
	{
		clear_square(*this, move.to, move.piece);
		set_square(*this, move.from, move.piece);

		const int captured_sq = (state.turn == Color::white)
			? static_cast<int>(move.to) - 8
			: static_cast<int>(move.to) + 8;
		set_square(*this, make_square(captured_sq), move.captured);
	}
	else if (move.is_promotion())
	{
		const PieceType promo_piece = promoted_piece_for_move(move.type, state.turn);

		clear_square(*this, move.to, promo_piece);
		set_square(*this, move.from, move.piece);

		if (move.captured != PieceType::none)
			set_square(*this, move.to, move.captured);
	}
	else
	{
		clear_square(*this, move.to, move.piece);
		set_square(*this, move.from, move.piece);

		if (move.captured != PieceType::none)
			set_square(*this, move.to, move.captured);
	}
}

MoveList& Position::get_legal_moves()
{
	return Generator::get_moves(*this, move_list);
}

MoveList& Position::get_pseudo_legal_moves()
{
	return Generator::get_moves(*this, move_list);
}

void Position::print() const
{
	constexpr char pieces[] = "PNBRQKpnbrqk";

	for (int rank = 7; rank >= 0; rank--)
	{
		std::cout << rank + 1 << "  ";
		for (int file = 0; file < 8; file++)
		{
			const int square = rank * 8 + file;
			const PieceType piece = mailbox[square];
			char found = (piece == PieceType::none) ? '.' : pieces[piece_val(piece)];
			std::cout << found << ' ';
		}
		std::cout << '\n';
	}
	std::cout << "   A B C D E F G H\n\n";
}

u64 Position::get_squares(Color color) const { return bitboards.occupied(color); }

bool is_number(char c) { return std::isdigit(static_cast<unsigned char>(c)); }

void Position::read_fen(const std::string& fen)
{
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
			const auto idx = 1ULL << square;
			bitboards[it->second] |= (idx);
			mailbox[square] = it->second;
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

	// Calculate zobrist key for this position
	zobrist_key = Zobrist::calculate_hash(*this);
}

PieceType Position::piece_at(Square sq) const
{
	if (sq == Square::none)
		return PieceType::none;
	return mailbox[static_cast<int>(sq)];
}

void Position::make_move(const std::string &token)
{
	if (token.size() != 4 && token.size() != 5)
		return;

	auto parse_square = [](char file_char, char rank_char) -> Square
		{
			file_char = static_cast<char>(std::tolower(static_cast<unsigned char>(file_char)));
			if (file_char < 'a' || file_char > 'h' || rank_char < '1' || rank_char > '8')
				return Square::none;
			return make_square(file_char - 'a', rank_char - '1');
		};

	const Square from = parse_square(token[0], token[1]);
	const Square to = parse_square(token[2], token[3]);
	if (from == Square::none || to == Square::none)
		return;

	char promotion_piece = '\0';
	if (token.size() == 5)
	{
		promotion_piece = static_cast<char>(std::tolower(static_cast<unsigned char>(token[4])));
		if (promotion_piece != 'q' && promotion_piece != 'r' && promotion_piece != 'b' && promotion_piece != 'n')
			return;
	}

	auto promotion_matches = [](MoveType type, char promo) -> bool
		{
			switch (promo)
			{
			case 'q':
				return type == MoveType::promotion_queen || type == MoveType::promotion_queen_capture;
			case 'r':
				return type == MoveType::promotion_rook || type == MoveType::promotion_rook_capture;
			case 'b':
				return type == MoveType::promotion_bishop || type == MoveType::promotion_bishop_capture;
			case 'n':
				return type == MoveType::promotion_knight || type == MoveType::promotion_knight_capture;
			default:
				return false;
			}
		};

	MoveList& legal_moves = get_legal_moves();
	for (int i = 0; i < static_cast<int>(legal_moves.count); ++i)
	{
		const Move& move = legal_moves[i];
		if (move.from != from || move.to != to)
			continue;

		if (token.size() == 5)
		{
			if (!move.is_promotion() || !promotion_matches(move.type, promotion_piece))
				continue;
		}
		else if (move.is_promotion())
		{
			continue;
		}

		MoveState state{};
		apply_move(move, state);
		return;
	}
}
