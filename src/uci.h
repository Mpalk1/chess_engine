#pragma once

#include "engine.h"
#include "position.h"
#include "types.h"

struct Uci
{
    Position position{};
    Engine engine{};

    void run();
    static u64 perft(Position& b, int depth);
};