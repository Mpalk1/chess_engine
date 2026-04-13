#pragma once
#include <array>
#include <cstdint>
#include "types.h"

using u64 = uint64_t;

// Zobrist hashing: random numbers for each piece type at each square
// plus random numbers for castling rights, en passant files, and side to move
class Zobrist
{
public:
    // Generate all zobrist keys from a seed
    static void initialize(u64 seed = 0x9D2C5680A3F47B1CULL);

    // Get the piece key arrays and other constants
    static std::array<u64, 768> piece_keys;           // 12 pieces * 64 squares
    static std::array<u64, 16> castling_keys;          // 16 castling combinations
    static std::array<u64, 9> enpassant_keys;          // 8 files + 1 for no en passant
    static u64 side_key;                                 // Side to move key

    // Calculate Zobrist hash for a position
    static u64 calculate_hash(const class Position& position);

private:
    static u64 pseudo_random(u64& seed);
};
