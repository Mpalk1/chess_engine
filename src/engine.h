#pragma once
#include <array>
#include <thread>
#include <atomic>
#include <chrono>

#include "position.h"
#include "transposition_table.h"
#include "search_stats.h"

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

constexpr std::array<int, 64> mirror_table(const std::array<int, 64>& table)
{
    std::array<int, 64> mirrored{};
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            mirrored[r * 8 + c] = table[(7 - r) * 8 + c];
    return mirrored;
}

struct Engine
{
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
    uint64_t last_info_time = 0;  // Last time we printed info line (ms)
    std::chrono::steady_clock::time_point hard_stop_time{};
    uint64_t soft_time_limit_ms = 0;
    bool use_time_limit = false;
    bool iteration_aborted = false;

    static int get_piece_value(Position& position, int i);
    bool should_stop_search() const;
    void search_worker(Position& position, SearchLimits limits);
    int calculate_time_budget_ms(const Position& position, const SearchLimits& limits) const;

    static constexpr std::array<int, 12> pieces_values{100, 320, 330, 500, 900, 0,
                                                       100, 320, 330, 500, 900, 0};

    static constexpr std::array<int, 64> white_pawn_table{
         0,   0,   0,   0,   0,   0,   0,   0,
        50,  50,  50,  50,  50,  50,  50,  50,
        10,  10,  20,  30,  30,  20,  10,  10,
         5,   5,  10,  25,  25,  10,   5,   5,
         0,   0,   0,  20,  20,   0,   0,   0,
         5,  -5, -10,   0,   0, -10,  -5,   5,
         5,  10,  10, -20, -20,  10,  10,   5,
         0,   0,   0,   0,   0,   0,   0,   0,
    };
    static constexpr std::array<int, 64> black_pawn_table = mirror_table(white_pawn_table);

    static constexpr std::array<int, 64> white_knight_table{
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -30,   0,  10,  15,  15,  10,   0, -30,
        -30,   5,  15,  20,  20,  15,   5, -30,
        -30,   0,  15,  20,  20,  15,   0, -30,
        -30,   5,  10,  15,  15,  10,   5, -30,
        -40, -20,   0,   5,   5,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50,
    };
    static constexpr std::array<int, 64> black_knight_table = mirror_table(white_knight_table);

    static constexpr std::array<int, 64> white_bishop_table{
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   5,   5,  10,  10,   5,   5, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   5,   0,   0,   0,   0,   5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20,
    };
    static constexpr std::array<int, 64> black_bishop_table = mirror_table(white_bishop_table);

    static constexpr std::array<int, 64> white_rook_table{
         0,   0,   0,   5,   5,   0,   0,   0,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
         5,  10,  10,  10,  10,  10,  10,   5,
         0,   0,   0,   0,   0,   0,   0,   0,
    };
    static constexpr std::array<int, 64> black_rook_table = mirror_table(white_rook_table);

    static constexpr std::array<int, 64> white_queen_table{
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,   5,   5,   5,   0, -10,
         -5,   0,   5,   5,   5,   5,   0,  -5,
          0,   0,   5,   5,   5,   5,   0,  -5,
        -10,   5,   5,   5,   5,   5,   0, -10,
        -10,   0,   5,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20,
    };
    static constexpr std::array<int, 64> black_queen_table = mirror_table(white_queen_table);

    static constexpr std::array<int, 64> white_king_table{
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
         20,  20,   0,   0,   0,   0,  20,  20,
         20,  30,  10,   0,   0,  10,  30,  20,
    };
    static constexpr std::array<int, 64> black_king_table = mirror_table(white_king_table);

    static constexpr std::array<std::array<int, 64>, 12> piece_square_tables{
        white_pawn_table,
        white_knight_table,
        white_bishop_table,
        white_rook_table,
        white_queen_table,
        white_king_table,
        black_pawn_table,
        black_knight_table,
        black_bishop_table,
        black_rook_table,
        black_queen_table,
        black_king_table,
    };
};