#include "board.h"
#include <cassert>
#include <iostream>
#include <bitset>
#include <unordered_map>
#include <vector>

#include "bitboard.h"

namespace
{
void AppendPawnPushMoves(std::vector<Move>& moves, const u64 targets,
                         const int advance, const PieceType piece,
                         const MoveType type)
{
  for (int to = 0; to < 64; ++to)
  {
    if ((targets & (1ULL << to)) == 0)
    {
      continue;
    }

    const int from = to - advance;
    moves.emplace_back(static_cast<Square>(from), static_cast<Square>(to), piece, type);
  }
}
}

Board::Board()
{
  for (int i = 0; i < 12; i++)
  {
    bitboards[i] = 0ULL;
  }
}

void Board::Clear()
{
  std::fill(std::begin(bitboards), std::end(bitboards),0ULL);
}


void Board::ReadFen(const std::string& fen)
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

    if (IsNumber(c))
    {
      ctr += c - '0';
      continue;
    }

    if (auto it = charToPiece.find(c); it != charToPiece.end())
    {
      const int rank = 7 - (ctr / 8);
      const int file = ctr % 8;
      const int square = rank * 8 + file;
      bitboards[PieceVal(it->second)] |= (1ULL << square);
      ctr++;
    }
  }
}

MoveList Board::GetLegalMoves()
{
  return MoveList{};
}

MoveList Board::GetPseudoLegalMoves()
{
  return MoveList{};
}

void Board::GeneratePawnMoves()
{
  move_list.clear();
  std::vector<Move> generated_moves;
  generated_moves.reserve(16);

  const auto empty_squares = GetEmptySquares();

  if (current_turn == Color::white)
  {
    const auto white_pawns = bitboards[PieceVal(PieceType::white_pawn)];
    const auto single_push = shift(white_pawns, Direction::north, 1) & empty_squares;
    const auto double_ready = shift(white_pawns & RANK_2, Direction::north, 1) & empty_squares;
    const auto double_push = shift(double_ready, Direction::north, 1) & empty_squares;

    AppendPawnPushMoves(generated_moves, single_push, 8,
                        PieceType::white_pawn, MoveType::normal);
    AppendPawnPushMoves(generated_moves, double_push, 16,
                        PieceType::white_pawn, MoveType::double_pawn_push);
  }
  else
  {
    const auto black_pawns = bitboards[PieceVal(PieceType::black_pawn)];
    const auto single_push = shift(black_pawns, Direction::south, 1) & empty_squares;
    const auto double_ready = shift(black_pawns & RANK_7, Direction::south, 1) & empty_squares;
    const auto double_push = shift(double_ready, Direction::south, 1) & empty_squares;

    AppendPawnPushMoves(generated_moves, single_push, -8,
                        PieceType::black_pawn, MoveType::normal);
    AppendPawnPushMoves(generated_moves, double_push, -16,
                        PieceType::black_pawn, MoveType::double_pawn_push);
  }

  for (const auto& move : generated_moves)
  {
    move_list.push(move);
  }
}
void Board::GenerateKnightMoves()
{

}
void Board::GenerateBishopMoves()
{

}
void Board::GenerateRookMoves()
{

}
void Board::GenerateQueenMoves()
{

}
void Board::GenerateKingMoves()
{

}

void Board::Print() const
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
  std::cout << "\n   A B C D E F G H\n";

  for (int i = 0; i < 12; i++)
    std::cout << "bitboard[" << i << "] = "
              << std::bitset<64>(bitboards[i]) << '\n';
}


bool IsNumber(char c)
{
  return std::isdigit(c);
}
