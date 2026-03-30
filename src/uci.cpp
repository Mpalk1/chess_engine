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
			int depth = -1;
			int movetime = -1;
			int wtime = -1, btime = -1;
			int winc = 0, binc = 0;

			while (iss >> token)
			{
				if (token == "depth") iss >> depth;
				else if (token == "movetime") iss >> movetime;
				else if (token == "wtime") iss >> wtime;
				else if (token == "btime") iss >> btime;
				else if (token == "winc") iss >> winc;
				else if (token == "binc") iss >> binc;

				// debug tools
				else if (token == "perft")
				{
					auto d = std::stoi(token);
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

			int time_for_move = 0;

			if (movetime != -1)
			{
				time_for_move = movetime;
			}
			else if (wtime != -1)
			{
				if (position.current_turn == Color::white)
					time_for_move = wtime / 30 + winc;
				else
					time_for_move = btime / 30 + binc;
			}

			// if (time_for_move < 50) depth = 3;
			// else if (time_for_move < 200) depth = 4;
			// else depth = 5;

			depth = 4;

			if (depth != -1)
			{
				engine.search_depth(position, depth);
			}
			else if (time_for_move > 0)
			{
				engine.search_time(position, time_for_move);
			}
			else
			{
				engine.search_depth(position, 4);
			}
		}
		else if (token == "quit")
		{
			std::cout << "quitting...\n";
			std::exit(0);
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
		b.make_move(m);
		nodes += perft(b, depth - 1);
		b.unmake_move(m);
	}
	return nodes;
}
