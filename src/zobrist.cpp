#include "zobrist.h"
#include "position.h"
#include <bit>


std::array<u64, 768> Zobrist::piece_keys{};
std::array<u64, 16> Zobrist::castling_keys{};
std::array<u64, 9> Zobrist::enpassant_keys{};
u64 Zobrist::side_key{};

void Zobrist::initialize(u64 seed)
{
    
    auto random = [](u64& state) -> u64 {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    };

    u64 state = seed;

    
    for (int i = 0; i < 768; ++i)
    {
        piece_keys[i] = random(state);
    }

    
    for (int i = 0; i < 16; ++i)
    {
        castling_keys[i] = random(state);
    }

    
    for (int i = 0; i < 9; ++i)
    {
        enpassant_keys[i] = random(state);
    }

    
    side_key = random(state);
}

u64 Zobrist::calculate_hash(const Position& position)
{
    u64 hash = 0;

    
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

    
    hash ^= castling_keys[position.state.castling_rights];

    
    if (position.state.en_passant_square != Square::None)
    {
        int ep_file = file(position.state.en_passant_square);
        hash ^= enpassant_keys[ep_file + 1]; 
    }

    
    if (position.state.turn == Color::Black)
    {
        hash ^= side_key;
    }

    return hash;
}
