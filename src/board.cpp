#include "board.h"
#include <cassert>
#include <iostream>
#include <bitset>
#include <unordered_map>

#include "bitboard.h"

Board::Board()
{
  init_knight_attacks();
}

void Board::clear()
{
  bitboards.clear();
}


void Board::read_fen(const std::string& fen)
{
  assert(!fen.empty());

  static const std::unordered_map<char, PieceType> charToPiece = {
    { 'P', PieceType::white_pawn   }, { 'N', PieceType::white_knight },
    { 'B', PieceType::white_bishop }, { 'R', PieceType::white_rook   },
    { 'Q', PieceType::white_queen  }, { 'K', PieceType::white_king   },
    { 'p', PieceType::black_pawn   }, { 'n', PieceType::black_knight },
    { 'b', PieceType::black_bishop }, { 'r', PieceType::black_rook   },
    { 'q', PieceType::black_queen  }, { 'k', PieceType::black_king   },
  };

  int ctr = 0;
  for (const char c : fen)
  {
    if (c == ' ') break; // todo: add info for castling rights, en passant
    if (c == '/') continue;

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
}

MoveList Board::get_legal_moves()
{
  return MoveList{};
}

MoveList Board::get_pseudo_legal_moves()
{
  return MoveList{};
}

void Board::generate_pawn_moves()
{
  move_list.clear();

  const auto empty_squares = get_empty_squares();

  if (current_turn == Color::white)
  {
    const auto white_pawns = bitboards[PieceType::white_pawn];
    const auto single_push = shift(white_pawns, Direction::north, 1) & empty_squares;
    const auto double_ready = shift(white_pawns & RANK_2, Direction::north, 1) & empty_squares;
    const auto double_push = shift(double_ready, Direction::north, 1) & empty_squares;

    move_list.add_pawn_move(single_push, 8, PieceType::white_pawn, MoveType::normal);
    move_list.add_pawn_move(double_push, 16, PieceType::white_pawn, MoveType::double_pawn_push);
  }
  else
  {
    const auto black_pawns = bitboards[PieceType::black_pawn];
    const auto single_push = shift(black_pawns, Direction::south, 1) & empty_squares;
    const auto double_ready = shift(black_pawns & RANK_7, Direction::south, 1) & empty_squares;
    const auto double_push = shift(double_ready, Direction::south, 1) & empty_squares;

    move_list.add_pawn_move(single_push, -8, PieceType::black_pawn, MoveType::normal);
    move_list.add_pawn_move(double_push, -16, PieceType::black_pawn, MoveType::double_pawn_push);
  }
}
void Board::generate_knight_moves()
{
  const auto friendly_squares = get_squares(current_turn == Color::white ? Color::white : Color::black);
  auto knights = bitboards[current_turn == Color::white ? PieceType::white_knight : PieceType::black_knight];
  while (knights) {
    int from_idx = std::countr_zero(knights.get());
    const Square from = static_cast<Square>(from_idx);

    const u64 moves = knight_attacks[from_idx] & ~friendly_squares;

    move_list.add_piece_moves(from, moves, current_turn == Color::white ? PieceType::white_knight : PieceType::black_knight);

    knights &= knights - 1; // clear the first bit set to 1 counting from LSB
  }
}
void Board::generate_bishop_moves()
{

}
void Board::generate_rook_moves()
{

}
void Board::generate_queen_moves()
{

}
void Board::generate_king_moves()
{

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

  // for (int i = 0; i < 12; i++)
  //   std::cout << "bitboard[" << i << "] = "
  //             << std::bitset<64>(bitboards[i]) << '\n';
}

u64 Board::get_squares(Color color) const
{
  return bitboards.occupied(color);
}

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
    attacks |= (b << 6)  & ~(FILE_G | FILE_H);
    attacks |= (b >> 10) & ~(FILE_G | FILE_H);
    attacks |= (b >> 6)  & ~(FILE_A | FILE_B);

    knight_attacks[s] = attacks;
  }
}

bool is_number(char c)
{
  return std::isdigit(c);
}
