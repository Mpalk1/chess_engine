#include <bitset>
#include <iostream>
#include "board.h"

int main()
{
  Board board{};
  board.read_fen(board.starting_fen);
  board.print();
  // board.current_turn = Color::black;
  board.generate_pawn_moves();
  board.generate_knight_moves();
  board.move_list.print();
}
