#pragma once

#include "move.h"
#include "types.h"
#include <vector>


enum class ScoreFlag : u8
{
    Exact = 0,      
    LowerBound = 1, 
    UpperBound = 2  
};


struct TTEntry
{
    u64 key = 0; 
    int score = 0;
    u8 depth = 0; 
    ScoreFlag flag = ScoreFlag::Exact;
    Move best_move{};

    bool is_valid() const
    {
        return key != 0;
    }
};


class TranspositionTable
{
public:
    explicit TranspositionTable(size_t mb_size = 64);
    ~TranspositionTable() = default;

    
    bool probe(u64 key, TTEntry& entry) const;

    
    void store(u64 key, int score, u8 depth, ScoreFlag flag, const Move& best_move);

    
    void clear();

    
    size_t size_bytes() const
    {
        return table.size() * sizeof(TTEntry);
    }

    
    size_t entry_count() const
    {
        return table.size();
    }

private:
    std::vector<TTEntry> table;
    size_t hash_mask;

    size_t index(u64 key) const;
};
