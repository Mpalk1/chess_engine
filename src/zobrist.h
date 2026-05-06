#pragma once

#include "types.h"
#include <array>

class Zobrist
{
public:
    static void initialize(u64 seed = 0x9D2C5680A3F47B1CULL);

    static std::array<u64, 768> piece_keys;
    static std::array<u64, 16> castling_keys;
    static std::array<u64, 9> enpassant_keys;
    static u64 side_key;

    static u64 calculate_hash(const class Position& position);

private:
    static u64 pseudo_random(u64& seed);
};
