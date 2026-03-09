#include "board.h"
#include <cassert>
#include <iostream>

#include <bitset>
#include <unordered_map>

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
      bitboards[PieceVal(it->second)-1] |= (1ULL << square); // -1 because PieceType::none is 0
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

void Board::GenetatePawnMoves()
{

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
