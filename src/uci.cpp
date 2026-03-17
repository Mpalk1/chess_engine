#include "uci.h"
#include <sstream>

void Uci::run()
{
	std::string line{};
	auto position_init = false;
	while (std::getline(std::cin, line))
	{
		std::istringstream iss(line);
		std::string token{};
		iss >> token;
		if (token == "uci")
		{
			std::cout << "id name PEngine\n";
			std::cout << "id author Maciej Palkowski\n";
			std::cout << "uciok\n";
		}
		else if (token == "isready")
		{
			std::cout << "readyok\n";
		}
		else if (token == "position")
		{
			iss >> token;
			if (token == "startpos")
			{
				board.read_fen(board.starting_fen);
				position_init = true;
			}
		}
		else if (token == "go" && position_init)
		{
			iss >> token;
			if (token == "perft")
			{
				iss >> token;
				if (is_number(token.c_str()[0]))
				{
					auto depth = std::stoi(token);
					for (int i{ 1 }; i <= depth; ++i)
						std::cout << "perft [" << i << "]: " << Uci::perft(board, i) << "\n";
				}
				

			}
		}
		
	}
}

u64 Uci::perft(Board& b, int depth)
{
	if (depth == 0)
		return 1;

	u64 nodes = 0;
	MoveList moves = b.get_legal_moves();

	for (size_t i = 0; i < moves.count; ++i)
	{
		Move m = moves[i];
		b.make_move(m);
		nodes += perft(b, depth - 1);
		b.unmake_move(m);
	}
	return nodes;
}

