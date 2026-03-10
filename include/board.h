#pragma once
#include <array>
#include <string>
#include "move_list.h"
#include "types.h"

struct Board
{
  std::array<u64, 12> bitboards{};
  MoveList move_list{};
  const std::string starting_fen
  {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
  Color current_turn = Color::white;
  Move previous_move{}; //for unmaking a move
  u64 GetEmptySquares() const
  {
    u64 occupied = 0ULL;
    for (const auto& bb : bitboards)
    {
      occupied |= bb;
    }
    return ~occupied;
  }
  Board();
  void Clear();
  void ReadFen(const std::string& fen);
  MoveList GetLegalMoves();
  MoveList GetPseudoLegalMoves();
  void Print() const;
// private:
  void GeneratePawnMoves();
  void GenerateKnightMoves();
  void GenerateBishopMoves();
  void GenerateRookMoves();
  void GenerateQueenMoves();
  void GenerateKingMoves();
};


bool IsNumber(char c);

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