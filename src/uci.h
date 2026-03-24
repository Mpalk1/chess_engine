#pragma once
#include <string>
#include "position.h"

struct Uci
{
	Position position{};

	void run();
	static u64 perft(Position& b, int depth);
};