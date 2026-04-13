#include "zobrist.h"
#include "position.h"
#include <bit>

// Static member initialization
std::array<u64, 768> Zobrist::piece_keys{};
std::array<u64, 16> Zobrist::castling_keys{};
std::array<u64, 9> Zobrist::enpassant_keys{};
u64 Zobrist::side_key{};

void Zobrist::initialize(u64 seed)
{
    // Simple pseudo-random number generator (xorshift64)
    auto random = [](u64& state) -> u64
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    };

    u64 state = seed;

    // Generate all piece keys
    for (int i = 0; i < 768; ++i)
    {
        piece_keys[i] = random(state);
    }

    // Generate castling keys
    for (int i = 0; i < 16; ++i)
    {
        castling_keys[i] = random(state);
    }

    // Generate en passant keys
    for (int i = 0; i < 9; ++i)
    {
        enpassant_keys[i] = random(state);
    }

    // Generate side to move key
    side_key = random(state);
}

u64 Zobrist::calculate_hash(const Position& position)
{
    u64 hash = 0;

    // Hash in all pieces
    for (int piece_idx = 0; piece_idx < 12; ++piece_idx)
    {
        u64 bitboard = position.bitboards[piece_idx].get();
        while (bitboard)
        {
            int sq = std::countr_zero(bitboard);
            hash ^= piece_keys[piece_idx * 64 + sq];
            bitboard &= bitboard - 1;
        }
    }

    // Hash in castling rights
    hash ^= castling_keys[position.castling_rights];

    // Hash in en passant square if valid
    if (position.en_passant_square != Square::none)
    {
        int ep_file = file(position.en_passant_square);
        hash ^= enpassant_keys[ep_file + 1]; // +1 because index 0 is no en passant
    }

    // Hash in side to move
    if (position.current_turn == Color::black)
    {
        hash ^= side_key;
    }

    return hash;
}
