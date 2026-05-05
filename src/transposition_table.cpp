#include "transposition_table.h"
#include <cmath>

TranspositionTable::TranspositionTable(size_t mb_size)
{
    
    size_t entry_size = sizeof(TTEntry);
    size_t num_entries = (mb_size * 1024 * 1024) / entry_size;

    
    size_t power_of_2 = 1;
    while (power_of_2 * 2 <= num_entries)
    {
        power_of_2 *= 2;
    }
    num_entries = power_of_2;
    hash_mask = num_entries - 1;

    
    table.resize(num_entries);
    clear();
}

bool TranspositionTable::probe(u64 key, TTEntry& entry) const
{
    if (table.empty())
        return false;

    size_t idx = index(key);
    const TTEntry& stored = table[idx];

    
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
