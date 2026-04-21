#include <sstream>
#include <cstdlib>
#include "uci.h"
#include "engine.h"

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
		else if (token == "ucinewgame")
		{
			engine.reset();
			position_init = false;
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
				iss >> token;
				if (token == "moves")
				{
					while (iss >> token)
					{
						position.make_move(token);
					}
				}
			}
			else if (token == "fen")
			{
				std::string fen{};
				std::string fen_field{};
				int fields_read = 0;
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
					iss >> token;
					if (token == "moves")
					{
						while (iss >> token)
						{
							position.make_move(token);
						}
					}
				}
				else
				{
					position_init = false;
				}
			}
		}
		else if (token == "go" && position_init)
		{
			SearchLimits limits{};
			limits.depth = 0;
			bool has_go_limit = false;

			while (iss >> token)
			{
				if (token == "depth")
				{
					iss >> limits.depth;
					has_go_limit = true;
				}
				else if (token == "movetime")
				{
					iss >> limits.movetime_ms;
					has_go_limit = true;
				}
				else if (token == "wtime")
				{
					iss >> limits.wtime_ms;
					has_go_limit = true;
				}
				else if (token == "btime")
				{
					iss >> limits.btime_ms;
					has_go_limit = true;
				}
				else if (token == "winc")
				{
					iss >> limits.winc_ms;
					has_go_limit = true;
				}
				else if (token == "binc")
				{
					iss >> limits.binc_ms;
					has_go_limit = true;
				}
				else if (token == "movestogo")
				{
					iss >> limits.movestogo;
					has_go_limit = true;
				}
				else if (token == "infinite")
				{
					limits.infinite = true;
					has_go_limit = true;
				}
				else if (token == "ponder") continue;

				// debug tools
				else if (token == "perft")
				{
					int d = 0;
					if (!(iss >> d))
						continue;
					size_t nodes = 0;
					for (int i{ 1 }; i <= d; ++i)
					{
						auto temp = perft(position, i);
						std::cout << "perft [" << i << "]: " << temp << "\n";
						if (i == d)
						{
							nodes += temp;
							std::cout << "Nodes searched: " << nodes << std::endl;
						}
					}
				}
				else if (token == "eval")
				{
					std::cout << "eval: " << engine.evaluate(position) << "\n";
					continue;
				}
			}

			if (!has_go_limit)
				limits.infinite = true;

			engine.search(position, limits);
		}
		else if (token == "quit")
		{
			std::cout << "quitting...\n";
			std::exit(0);
		}
		else if (token == "print")
		{
			std::cout << "Board state:\n";
			position.print();
			std::cout << "Moves:\n";
			position.move_list.print();
		}
		else if (token == "stop" && position_init) engine.stop();
		else
		{
			std::cout << "unknown command : " << token << std::endl;
		}
		std::cout << std::flush;
	}
}

u64 Uci::perft(Position &b, int depth) {
	if (depth == 0)
		return 1;

	u64 nodes = 0;
	MoveList moves = b.get_legal_moves();

	for (auto i = 0; i < moves.count; ++i)
	{
		Move m = moves[i];
		MoveState state{};
		b.apply_move(m, state);
		nodes += perft(b, depth - 1);
		b.undo_move(m, state);
	}
	return nodes;
}
