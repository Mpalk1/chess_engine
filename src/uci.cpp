#include "uci.h"
#include <sstream>
#include "tracy/Tracy.hpp"

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
				position.read_fen(position.starting_fen);
				position_init = true;
			}
			else if (token == "fen")
			{
				std::string fen{};
				std::string fen_field{};
				int fields_read = 0;

				// UCI `position fen` expects exactly 6 space-separated FEN fields.
				for (; fields_read < 6 && (iss >> fen_field); ++fields_read)
				{
					if (!fen.empty())
						fen += " ";
					fen += fen_field;
				}

				if (fields_read == 6)
				{
					position.read_fen(fen);
					position_init = true;
				}
				else
				{
					position_init = false;
				}
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
					size_t nodes = 0;
					for (int i{ 1 }; i <= depth; ++i)
					{
						auto temp = perft(position, i);
						std::cout << "perft [" << i << "]: " << temp << "\n";
						if (i == depth)
						{
							nodes += temp;
							std::cout << "Nodes searched: " << nodes << std::endl;
						}
					}

				}
				

			}
		}
		std::cout << std::flush;
	}
}

u64 Uci::perft(Position &b, int depth) {
	ZoneScoped;
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

