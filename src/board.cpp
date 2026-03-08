#include "board.h"

Board::Board() {
  for (int i = 0; i < 12; i++) {
    bitboards[i] = 0ULL;
  }
}
