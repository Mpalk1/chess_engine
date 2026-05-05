#pragma once

#include "position.h"
#include "search_stats.h"
#include "transposition_table.h"
#include <array>
#include <atomic>
#include <chrono>
#include <thread>

struct SearchLimits
{
    int depth = 64;
    int movetime_ms = -1;
    int wtime_ms = -1;
    int btime_ms = -1;
    int winc_ms = 0;
    int binc_ms = 0;
    int movestogo = -1;
    bool infinite = false;
};

class Engine
{
public:
    Engine();
    ~Engine();

    void search(Position& position, const SearchLimits& limits);
    void search(Position& position, int depth);
    void search_time(Position& position, int time_ms);
    int evaluate(Position& position);
    int minimax(Position& position, int depth, int alpha, int beta, std::vector<Move>& pv, int ply);
    void stop();
    void reset();
    void clear_transposition_table() { transposition_table.clear(); }
    void output_info(bool force = false) const;

private:
    unsigned int thread_count{};
    std::thread worker{};

    std::atomic<bool> should_work{true};
    std::atomic<bool> work_done{false};
    Move best_move{};
    TranspositionTable transposition_table{64};
    SearchStats stats;
    uint64_t last_info_time = 0; 
    std::chrono::steady_clock::time_point hard_stop_time{};
    uint64_t soft_time_limit_ms = 0;
    bool use_time_limit = false;
    bool iteration_aborted = false;

    static int get_piece_value(Position& position, int i);
    bool should_stop_search() const;
    void search_worker(Position& position, SearchLimits limits);
    int calculate_time_budget_ms(const Position& position, const SearchLimits& limits) const;
};