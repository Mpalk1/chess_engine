
#include <iostream>
#include <string>
#include <cstring>
#include "board.h"
#include "types.h"

// Parse "e2e4" into from/to squares. Returns false if the input is invalid.
static bool parse_move(const std::string& input, Square& from, Square& to)
{
  if (input.size() < 4)
    return false;

  const int from_file = input[0] - 'a';
  const int from_rank = input[1] - '1';
  const int to_file   = input[2] - 'a';
  const int to_rank   = input[3] - '1';

  if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
      to_file   < 0 || to_file   > 7 || to_rank   < 0 || to_rank   > 7)
    return false;

  from = make_square(from_file, from_rank);
  to   = make_square(to_file,   to_rank);
  return true;
}

int main(int argc, char** argv)
{
  Board board{};
  board.read_fen(board.starting_fen);

  if (argc > 1 && strcmp(argv[1], "-v") == 0)
  {
    board.print();
    std::string input;
    while (true)
    {
      auto& pseudo_moves = board.get_pseudo_legal_moves();

      std::cout << (board.current_turn == Color::white ? "White" : "Black") << " to move: ";
      if (!std::getline(std::cin, input))
        break;

      if (input == "quit" || input == "q")
        break;

      Square from, to;
      if (!parse_move(input, from, to))
      {
        std::cout << "Invalid input. Use format e2e4.\n";
        continue;
      }

      bool legal = false;
      for (size_t i = 0; i < pseudo_moves.count; ++i)
      {
        if (pseudo_moves[i].from == from && pseudo_moves[i].to == to)
        {
          legal = true;
          break;
        }
      }

      if (!legal)
      {
        std::cout << "Illegal move. Try again.\n";
        continue;
      }

      board.make_move(from, to);
      board.print();
    }
  }
}
