#pragma once
#include <cstdint>
#include <vector>
#include <cstring>
#include "types.h"
#include "move.h"

// Flag indicating what the score in a TT entry represents
enum class ScoreFlag : u8
{
    Exact = 0,          // True minimax value
    LowerBound = 1,     // Score >= stored value (beta cutoff)
    UpperBound = 2      // Score <= stored value (alpha cutoff)
};

// Single transposition table entry
struct TTEntry
{
    u64 key = 0;                    // Zobrist hash key
    int score = 0;                  // Evaluation score
    u8 depth = 0;                   // Search depth when stored
    ScoreFlag flag = ScoreFlag::Exact;
    Move best_move{};               // Best move found at this position

    bool is_valid() const { return key != 0; }
};

// Transposition table - stores positions and their evaluations
class TranspositionTable
{
public:
    explicit TranspositionTable(size_t mb_size = 64);
    ~TranspositionTable() = default;

    // Probe the table for an entry
    bool probe(u64 key, TTEntry& entry) const;

    // Store an entry in the table
    void store(u64 key, int score, u8 depth, ScoreFlag flag, const Move& best_move);

    // Clear the entire table
    void clear();

    // Get the size of the table in bytes
    size_t size_bytes() const { return table.size() * sizeof(TTEntry); }

    // Get entry count
    size_t entry_count() const { return table.size(); }

private:
    std::vector<TTEntry> table;
    size_t hash_mask;

    size_t index(u64 key) const;
};
