#pragma once
#include <array>
#include "move.h"


struct MoveList
{
  static constexpr int MaxMoves{256};
  std::array<Move, 256> moves{};
  size_t count{};

  void push(const Move& move) { moves[count++] = move; }
  void clear()
  {
    count = 0;
    moves.fill(Move());
  }
  const Move& begin() const { return moves[0]; }
  const Move& end() const { return moves[count]; }
  Move& operator[](int idx) { return moves[idx]; }
};
