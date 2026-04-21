#include "engine.h"
#include "generator.h"
#include "zobrist.h"
#include <algorithm>
#include <bit>
#include <limits>
#include <chrono>

namespace
{
int move_piece_value(PieceType piece)
{
    switch (piece)
    {
    case PieceType::white_pawn:
    case PieceType::black_pawn:
        return 100;
    case PieceType::white_knight:
    case PieceType::black_knight:
        return 320;
    case PieceType::white_bishop:
    case PieceType::black_bishop:
        return 330;
    case PieceType::white_rook:
    case PieceType::black_rook:
        return 500;
    case PieceType::white_queen:
    case PieceType::black_queen:
        return 900;
    default:
        return 0;
    }
}

bool same_move(const Move& lhs, const Move& rhs)
{
    return lhs.from == rhs.from && lhs.to == rhs.to && lhs.type == rhs.type && lhs.piece == rhs.piece;
}

bool contains_move(const MoveList& moves, const Move& needle)
{
    for (size_t i = 0; i < moves.count; ++i)
    {
        if (same_move(moves[i], needle))
            return true;
    }
    return false;
}

int move_order_score(const Move& move, const Move* tt_move)
{
    if (tt_move && same_move(move, *tt_move))
        return 1'000'000;

    int score = 0;

    if (move.is_promotion())
    {
        score += 100'000;
        switch (move.type)
        {
        case MoveType::promotion_queen:
        case MoveType::promotion_queen_capture:
            score += 4'000;
            break;
        case MoveType::promotion_rook:
        case MoveType::promotion_rook_capture:
            score += 3'000;
            break;
        case MoveType::promotion_bishop:
        case MoveType::promotion_bishop_capture:
            score += 2'000;
            break;
        case MoveType::promotion_knight:
        case MoveType::promotion_knight_capture:
            score += 1'000;
            break;
        default:
            break;
        }
    }

    if (move.is_capture())
        score += 10'000 + move_piece_value(move.captured) - move_piece_value(move.piece);

    if (move.is_check())
        score += 500;

    if (move.is_castle())
        score += 50;

    return score;
}

std::vector<Move> ordered_moves(const MoveList& move_list, const Move* tt_move)
{
    std::vector<Move> moves(move_list.begin(), move_list.begin() + move_list.count);
    std::stable_sort(moves.begin(), moves.end(), [tt_move](const Move& lhs, const Move& rhs)
    {
        return move_order_score(lhs, tt_move) > move_order_score(rhs, tt_move);
    });
    return moves;
}

std::vector<Move> legal_prefix_from_root(const Position& root, const std::vector<Move>& pv)
{
    std::vector<Move> legal_pv;
    legal_pv.reserve(pv.size());

    Position pos = root;
    for (const Move& move : pv)
    {
        MoveList legal_moves{};
        Generator::get_moves(pos, legal_moves);
        bool found = false;
        Move matched{};
        for (size_t i = 0; i < legal_moves.count; ++i)
        {
            if (same_move(legal_moves[i], move))
            {
                found = true;
                matched = legal_moves[i];
                break;
            }
        }

        if (!found)
            break;

        legal_pv.push_back(matched);
        MoveState state{};
        pos.apply_move(matched, state);
    }

    return legal_pv;
}
}

Engine::Engine()
{
    thread_count = std::max(1u, std::thread::hardware_concurrency());
    // Initialize zobrist hashing with a seed
    Zobrist::initialize(0x9D2C5680A3F47B1CULL);
}

Engine::~Engine()
{
    stop();
}

void Engine::search(Position& position, const SearchLimits& limits)
{
    if (worker.joinable())
    {
        should_work.store(false);
        worker.join();
    }
    best_move = Move{};
    should_work.store(true);
    work_done.store(false);
    clear_transposition_table();

    // Initialize search stats
    stats = SearchStats();
    stats.start_time = std::chrono::steady_clock::now();
    last_info_time = 0;
    use_time_limit = false;
    soft_time_limit_ms = 0;
    iteration_aborted = false;

    worker = std::thread{&Engine::search_worker, this, std::ref(position), limits};
}

void Engine::search(Position& position, int depth)
{
    SearchLimits limits{};
    limits.depth = std::max(1, depth);
    search(position, limits);
}

void Engine::search_time(Position& position, int time_ms)
{
    SearchLimits limits{};
    limits.movetime_ms = std::max(1, time_ms);
    search(position, limits);
}

int Engine::calculate_time_budget_ms(const Position& position, const SearchLimits& limits) const
{
    if (limits.movetime_ms > 0)
        return limits.movetime_ms;

    const int time_left = (position.current_turn == Color::white) ? limits.wtime_ms : limits.btime_ms;
    const int increment = (position.current_turn == Color::white) ? limits.winc_ms : limits.binc_ms;

    if (time_left <= 0)
        return 0;

    const int moves_to_go = (limits.movestogo > 0) ? limits.movestogo : 30;
    const int reserve = std::max(20, time_left / 50);
    const int safe_left = std::max(1, time_left - reserve);

    int budget = safe_left / std::max(1, moves_to_go);
    budget += (increment * 3) / 4;

    return std::max(10, std::min(budget, safe_left));
}

bool Engine::should_stop_search() const
{
    if (!should_work.load())
        return true;

    if (!use_time_limit)
        return false;

    return std::chrono::steady_clock::now() >= hard_stop_time;
}

void Engine::search_worker(Position& position, SearchLimits limits)
{
    constexpr int INF = 1'000'000;

    MoveList root_move_list{};
    Generator::get_moves(position, root_move_list);
    if (root_move_list.count == 0)
    {
        work_done.store(true);
        std::cout << "bestmove 0000" << std::endl;
        return;
    }

    best_move = root_move_list[0];

    const int budget_ms = calculate_time_budget_ms(position, limits);
    use_time_limit = !limits.infinite && budget_ms > 0;

    if (limits.depth <= 0)
        limits.depth = (use_time_limit || limits.infinite) ? 256 : 64;

    if (use_time_limit)
    {
        const auto now = std::chrono::steady_clock::now();
        hard_stop_time = now + std::chrono::milliseconds(std::max(1, budget_ms));
        soft_time_limit_ms = static_cast<uint64_t>(budget_ms * 8 / 10);
    }

    for (int depth = 1; depth <= limits.depth; ++depth)
    {
        if (should_stop_search())
            break;

        iteration_aborted = false;
        int alpha = -INF;
        int beta = INF;
        int best_val = std::numeric_limits<int>::min();
        Move depth_best_move = best_move;
        std::vector<Move> depth_best_pv;

        MoveList depth_root_moves{};
        Generator::get_moves(position, depth_root_moves);

        TTEntry root_tt_entry{};
        const Move* tt_move = nullptr;
        if (transposition_table.probe(position.zobrist_key, root_tt_entry) &&
            contains_move(depth_root_moves, root_tt_entry.best_move))
        {
            tt_move = &root_tt_entry.best_move;
        }

        auto moves = ordered_moves(depth_root_moves, tt_move);
        for (const Move& move : moves)
        {
            if (should_stop_search())
            {
                iteration_aborted = true;
                break;
            }

            MoveState state{};
            position.apply_move(move, state);

            std::vector<Move> pv;
            const int val = -minimax(position, depth - 1, -beta, -alpha, pv, 1);
            position.undo_move(move, state);

            if (iteration_aborted)
                break;

            if (val > best_val)
            {
                best_val = val;
                depth_best_move = move;
                depth_best_pv.clear();
                depth_best_pv.push_back(move);
                depth_best_pv.insert(depth_best_pv.end(), pv.begin(), pv.end());
            }

            alpha = std::max(alpha, best_val);
            if (alpha >= beta)
                break;
        }

        if (iteration_aborted)
            break;

        if (best_val != std::numeric_limits<int>::min())
        {
            best_move = depth_best_move;
            stats.score = best_val;
            stats.pv = legal_prefix_from_root(position, depth_best_pv);
        }

        // Keep UCI info streaming at every fully completed depth.
        stats.depth = depth;
        output_info(true);

        if (use_time_limit && stats.elapsed_ms() >= soft_time_limit_ms)
            break;
    }

    work_done.store(true);
    std::cout << "bestmove " << move_to_uci(best_move) << std::endl;
}

int Engine::get_piece_value(Position& position, int i)
{
    int eval{0};
    auto piece{position.bitboards[i].get()};
    //count pieces values
    const auto piece_count = std::popcount(piece);
    if (piece_count == 0) return 0;
    const auto piece_val = pieces_values.at(i) * piece_count;
    eval += piece_val;
    //temporary: bonus for piece placement
    while (piece)
    {
        int sq = std::countr_zero(piece);
        eval += piece_square_tables[i][63 - sq];
        piece &= piece - 1;
    }
    return eval;
}

int Engine::evaluate(Position& position)
{
    int eval{0};

    for (int i{0}; i < 6; ++i)
        eval += get_piece_value(position, i);

    for (int i{6}; i < 12; ++i)
        eval -= get_piece_value(position, i);

    eval += static_cast<int>(position.get_pseudo_legal_moves().count * 10);

    //to add:
    //count mobility of pieces
    //count squares attacked

    return position.current_turn == Color::white ? eval : -eval; // flipping the sign because searching is from the current players perspective
}

int Engine::minimax(Position& position, int depth, int alpha, int beta, std::vector<Move>& pv, int ply)
{
    if ((stats.nodes & 1023ULL) == 0 && should_stop_search())
    {
        iteration_aborted = true;
        return 0;
    }

    // Track nodes
    stats.nodes++;
    stats.seldepth = std::max(stats.seldepth, ply);

    const int alpha_orig = alpha;

    // Probe transposition table. Only trust entries with a legal stored move.
    TTEntry tt_entry;
    MoveList legal_moves{};
    Generator::get_moves(position, legal_moves);
    const bool has_tt = transposition_table.probe(position.zobrist_key, tt_entry);
    const bool tt_move_legal = has_tt && contains_move(legal_moves, tt_entry.best_move);
    if (has_tt && tt_move_legal)
    {
        // Check if the stored depth is sufficient
        if (tt_entry.depth >= depth)
        {
            // Return the score based on the flag
            if (tt_entry.flag == ScoreFlag::Exact)
            {
                pv.clear();
                pv.push_back(tt_entry.best_move);
                return tt_entry.score;
            }
            if (tt_entry.flag == ScoreFlag::LowerBound)
                alpha = std::max(alpha, tt_entry.score);
            else // UpperBound
                beta = std::min(beta, tt_entry.score);

            // Alpha-beta pruning
            if (alpha >= beta)
            {
                pv.clear();
                pv.push_back(tt_entry.best_move);
                return tt_entry.score;
            }
        }
    }

    if (depth == 0)
    {
        pv.clear();
        return evaluate(position);
    }

    int best = std::numeric_limits<int>::min();
    ScoreFlag flag = ScoreFlag::UpperBound; // Assume upper bound
    Move best_move_found{};
    std::vector<Move> best_pv;

    TTEntry move_order_entry{};
    const Move* tt_move = nullptr;
    if (transposition_table.probe(position.zobrist_key, move_order_entry) &&
        contains_move(legal_moves, move_order_entry.best_move))
    {
        tt_move = &move_order_entry.best_move;
    }

    auto moves = ordered_moves(legal_moves, tt_move);
    if (moves.empty())
    {
        pv.clear();
        return evaluate(position);
    }

    for (const Move& move : moves)
    {
        if ((stats.nodes & 1023ULL) == 0 && should_stop_search())
        {
            iteration_aborted = true;
            break;
        }

        MoveState state{};
        position.apply_move(move, state);

        std::vector<Move> child_pv;
        int val = -minimax(position, depth - 1, -beta, -alpha, child_pv, ply + 1);
        position.undo_move(move, state);

        if (iteration_aborted)
            break;

        if (val > best)
        {
            best = val;
            best_move_found = move;
            best_pv.clear();
            best_pv.push_back(move);
            best_pv.insert(best_pv.end(), child_pv.begin(), child_pv.end());
        }

        alpha = std::max(alpha, best);
        if (alpha >= beta)
        {
            flag = ScoreFlag::LowerBound; // Beta cutoff - lower bound
            break;
        }
    }

    if (iteration_aborted)
        return 0;

    // Determine the flag for storing
    if (best <= alpha_orig)
        flag = ScoreFlag::UpperBound;
    else if (best >= beta)
        flag = ScoreFlag::LowerBound;
    else
        flag = ScoreFlag::Exact;

    // Store in transposition table
    transposition_table.store(position.zobrist_key, best, depth, flag, best_move_found);

    pv = best_pv;
    return best;
}

void Engine::output_info(bool force) const
{
    // Only output if enough time has passed (throttle to avoid spam)
    uint64_t current_time = stats.elapsed_ms();
    if (!force && current_time - last_info_time < 100)
        return;  // Wait at least 100ms between info lines
    
    const_cast<Engine*>(this)->last_info_time = current_time;
    std::cout << stats.to_info_string() << std::endl;
}

void Engine::stop()
{
    should_work.store(false);
    if (worker.joinable())
        worker.join();
}

void Engine::reset()
{
    should_work.store(false);
    if (worker.joinable()) worker.join();
    best_move = Move{};
    work_done.store(false);
    should_work.store(true);
}
