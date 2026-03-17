#pragma once
#include <string>
#include "board.h"

struct Uci
{
	Board board{};

	void run();
	static u64 perft(Board& b, int depth);
};