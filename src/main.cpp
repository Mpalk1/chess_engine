#include <string>
#include "types.h"
#include "uci.h"

#include "tracy/Tracy.hpp"

// Parse "e2e4" into from/to squares. Returns false if the input is invalid.
static bool parse_move(const std::string& input, Square& from, Square& to)
{
	if (input.size() < 4)
		return false;

	const int from_file = input[0] - 'a';
	const int from_rank = input[1] - '1';
	const int to_file = input[2] - 'a';
	const int to_rank = input[3] - '1';

	if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 || to_file < 0 || to_file > 7 || to_rank < 0 ||
		to_rank > 7)
		return false;

	from = make_square(from_file, from_rank);
	to = make_square(to_file, to_rank);
	return true;
}

int main()
{
	Uci uci{};
	uci.run();
	//Board board{};
	//board.read_fen(board.starting_fen);
	//board.print();
	//for (int i{ 1 }; i <= 5; ++i)
	//	std::cout << "perft [" << i << "]: " << Uci::perft(board, i) << "\n";
	//std::cin.get();
	//std::string input;
	//while (true)
	//{
	//  auto &pseudo_moves = board.get_pseudo_legal_moves();

	//  std::cout << (board.current_turn == Color::white ? "White" : "Black") << " to move: ";
	//  if (!std::getline(std::cin, input))
	//    break;

	//  if (input == "quit" || input == "q")
	//    break;

	//  Square from, to;
	//  if (!parse_move(input, from, to))
	//  {
	//    std::cout << "Invalid input. Use format e2e4.\n";
	//    continue;
	//  }

	//  const Move *matched = nullptr;
	//  for (size_t i = 0; i < pseudo_moves.count; ++i)
	//  {
	//    if (pseudo_moves[i].from == from && pseudo_moves[i].to == to)
	//    {
	//      matched = &pseudo_moves[i];
	//      break;
	//    }
	//  }

	//  if (!matched)
	//  {
	//    std::cout << "Illegal move. Try again.\n";
	//    continue;
	//  }

	//  board.make_move(*matched);
	//  board.print();
	//}
}
