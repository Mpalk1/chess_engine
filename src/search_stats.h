#pragma once
#include <cstdint>
#include <chrono>
#include <string>
#include <vector>
#include "move.h"

struct SearchStats
{
    int depth = 0;                          // Current depth being searched
    int seldepth = 0;                       // Max selective depth reached
    int score = 0;                          // Score in centipawns
    bool is_mate = false;                   // Whether score is a mate score
    int mate_moves = 0;                     // Moves to mate (if is_mate)
    uint64_t nodes = 0;                     // Total nodes searched
    std::chrono::steady_clock::time_point start_time;
    std::vector<Move> pv;                   // Principal variation (best line)

    // Get elapsed time in milliseconds
    uint64_t elapsed_ms() const
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    }

    // Get nodes per second
    uint64_t nps() const
    {
        uint64_t time_ms = elapsed_ms();
        if (time_ms == 0) return 0;
        return (nodes * 1000) / time_ms;
    }

    // Get transposition table usage as per mille (0-1000)
    int hashfull() const { return 0; }  // Will be filled in by engine

    // Format as UCI info line
    std::string to_info_string() const
    {
        std::string result = "info";

        if (depth > 0)
            result += " depth " + std::to_string(depth);

        if (seldepth > 0)
            result += " seldepth " + std::to_string(seldepth);

        if (is_mate)
            result += " score mate " + std::to_string(mate_moves);
        else
            result += " score cp " + std::to_string(score);

        result += " nodes " + std::to_string(nodes);
        result += " nps " + std::to_string(nps());
        result += " time " + std::to_string(elapsed_ms());

        // Principal variation
        if (!pv.empty())
        {
            result += " pv";
            for (const auto& move : pv)
            {
                result += " " + move_to_uci(move);
            }
        }

        return result;
    }
};
