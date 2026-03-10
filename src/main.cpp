#include <bitset>
#include <iostream>
#include "board.h"

int main()
{
  Board board{};
  board.ReadFen(board.starting_fen);
  // board.Print();
  board.GeneratePawnMoves();
}
