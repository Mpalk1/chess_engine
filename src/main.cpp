
#include "bitboard.h"
#include "board.h"
#include "types.h"

int main()
{
  Board board{};
  board.read_fen(board.starting_fen);
  // board.current_turn = Color::black;
  // board.bitboards[PieceType::white_king].clear();
  // auto x = 1ULL << 35;
  board.bitboards[PieceType::white_king].shift_inplace(Direction::north, 3);
  board.print();
  board.generate_pawn_moves();
  board.generate_knight_moves();
  board.generate_king_moves();
  board.move_list.print();
  const auto& ref = std::addressof(board);
}
