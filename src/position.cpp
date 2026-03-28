#include "position.h"
#include "bitboard.h"
#include "generator.h"
#include "move_list.h"
#include "types.h"
#include <cassert>
#include <cctype>
#include <iostream>
#include <string_view>
#include <unordered_map>

Position::Position() = default;

void Position::clear()
{
	bitboards.clear();
	move_list.clear();
	current_turn = Color::white;
	castling_rights = 0;
	en_passant_square = Square::none;
	halfmove_clock = 0;
	fullmove_number = 1;
	previous_move = Move{};
}

void Position::make_move(Square from, Square to)
{
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

void Position::unmake_move(Move& move)
{
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

void Position::unmake_move(Square from, Square to)
{
	if (previous_move.from == from && previous_move.to == to)
	{
		unmake_move(previous_move);
		previous_move = Move{};
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

void Position::make_move(Move& move)
{
	move.prev_castling_rights = castling_rights;
	move.prev_enpassant_sq = en_passant_square;
	move.prev_halfmove_clock = halfmove_clock;
	move.prev_fullmove_number = fullmove_number;
	move.prev_turn = current_turn;

	make_move(static_cast<const Move&>(move));
}

void Position::make_move(const Move& move)
{
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

		make_move(move);
		return;
	}
}
