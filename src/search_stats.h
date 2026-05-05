#pragma once

#include "move.h"
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

struct SearchStats
{
    int depth = 0;        
    int seldepth = 0;     
    int score = 0;        
    bool is_mate = false; 
    int mate_moves = 0;   
    uint64_t nodes = 0;   
    std::chrono::steady_clock::time_point start_time;
    std::vector<Move> pv; 

    
    uint64_t elapsed_ms() const
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    }

    
    uint64_t nps() const
    {
        uint64_t time_ms = elapsed_ms();
        if (time_ms == 0)
            return 0;
        return (nodes * 1000) / time_ms;
    }

    
    int hashfull() const
    {
        return 0;
    } 

    
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
