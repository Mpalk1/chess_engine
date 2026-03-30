#pragma once
#include "position.h"
#include "engine.h"

struct Uci
{
	Position position{};
	Engine engine{};

	void run();
	static u64 perft(Position& b, int depth);
};