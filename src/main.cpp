#include <bitset>
#include <iostream>
#include "board.h"

int main()
{
  Board board{};
  board.ReadFen(board.starting_fen);
  board.Print();
  board.ReadFen("r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 b - - 1 23");
  board.Print();
  return 0;
}
