#include "transposition_table.h"
#include <cmath>

TranspositionTable::TranspositionTable(size_t mb_size)
{
    // Calculate number of entries based on MB size
    // Each entry is roughly 24-32 bytes (key, score, depth, flag, move struct)
    size_t entry_size = sizeof(TTEntry);
    size_t num_entries = (mb_size * 1024 * 1024) / entry_size;
    
    // Round down to nearest power of 2 for efficient masking
    size_t power_of_2 = 1;
    while (power_of_2 * 2 <= num_entries)
    {
        power_of_2 *= 2;
    }
    num_entries = power_of_2;
    hash_mask = num_entries - 1;

    // Initialize table with empty entries
    table.resize(num_entries);
    clear();
}

bool TranspositionTable::probe(u64 key, TTEntry& entry) const
{
    if (table.empty())
        return false;

    size_t idx = index(key);
    const TTEntry& stored = table[idx];

    // Check if entry matches the key
    if (stored.key == key && stored.is_valid())
    {
        entry = stored;
        return true;
    }
    return false;
}

void TranspositionTable::store(u64 key, int score, u8 depth, ScoreFlag flag, const Move& best_move)
{
    if (table.empty())
        return;

    size_t idx = index(key);
    TTEntry& entry = table[idx];

    // Always replace if new entry is deeper or it's a new position
    // This implements "always replace" strategy for simplicity
    if (depth >= entry.depth || entry.key != key)
    {
        entry.key = key;
        entry.score = score;
        entry.depth = depth;
        entry.flag = flag;
        entry.best_move = best_move;
    }
}

void TranspositionTable::clear()
{
    for (auto& entry : table)
    {
        entry.key = 0;
        entry.score = 0;
        entry.depth = 0;
        entry.flag = ScoreFlag::Exact;
        entry.best_move = Move{};
    }
}

size_t TranspositionTable::index(u64 key) const
{
    return key & hash_mask;
}
