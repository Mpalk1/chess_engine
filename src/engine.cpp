#include "engine.h"
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
}

Engine::Engine()
{
    thread_count = std::max(1u, std::thread::hardware_concurrency());
    // Initialize zobrist hashing with a seed
    Zobrist::initialize(0x9D2C5680A3F47B1CULL);
}
// temporarily only launch one thread
void Engine::search(Position &position, int depth)
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
    
                                                        //a ref for now because its only 1 thread
    worker = std::thread{&Engine::search_depth, this, std::ref(position), depth, -1000, 1000};

}

void Engine::search(Position &position, double time)
{
    if (worker.joinable())
    {
        should_work.store(false);
        worker.join();
    }
    best_move = Move{};
    should_work.store(true);
    work_done.store(false);
    //a ref for now because its only 1 thread
    worker = std::thread{&Engine::search_time, this, std::ref(position), time};

}

void Engine::search_depth(Position& position, int depth, int alpha, int beta)
{
    int best_val = std::numeric_limits<int>::min();

    TTEntry root_tt_entry{};
    const Move* tt_move = nullptr;
    if (transposition_table.probe(position.zobrist_key, root_tt_entry))
        tt_move = &root_tt_entry.best_move;

    auto moves = ordered_moves(position.get_legal_moves(), tt_move);
    std::vector<Move> best_pv;

    for (Move move : moves)
    {
        if (!should_work.load()) break;
        MoveState state{};
        position.apply_move(move, state);
        
        std::vector<Move> pv;
        int val = -minimax(position, depth - 1, -beta, -alpha, pv);
        position.undo_move(move, state);

        if (val > best_val)
        {
            best_val = val;
            best_move = move;
            
            // Update best PV
            best_pv.clear();
            best_pv.push_back(move);
            best_pv.insert(best_pv.end(), pv.begin(), pv.end());
        }

        alpha = std::max(alpha, best_val);
        if (alpha >= beta) break;
    }

    // Update stats with final depth results
    stats.depth = depth;
    stats.score = best_val;
    stats.pv = best_pv;
    output_info(true);

    work_done.store(true);
    std::cout << "bestmove " << square_to_string(best_move.from) << square_to_string(best_move.to) << std::endl;
}

void Engine::search_time(Position& position, int depth)
{
    return;
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

int Engine::minimax(Position& position, int depth, int alpha, int beta, std::vector<Move>& pv)
{
    // Track nodes
    stats.nodes++;

    // Probe transposition table
    TTEntry tt_entry;
    if (transposition_table.probe(position.zobrist_key, tt_entry))
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
    if (transposition_table.probe(position.zobrist_key, move_order_entry))
        tt_move = &move_order_entry.best_move;

    auto moves = ordered_moves(position.get_legal_moves(), tt_move);
    for (Move move : moves)
    {
        if (!should_work.load()) break;
        MoveState state{};
        position.apply_move(move, state);
        
        std::vector<Move> child_pv;
        int val = -minimax(position, depth - 1, -beta, -alpha, child_pv);
        position.undo_move(move, state);

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

    // Determine the flag for storing
    if (best > alpha && best < beta)
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
    if (!work_done.load()) 
    {
        // Search was interrupted, wait and print bestmove
        if (worker.joinable()) worker.join();
        std::cout << "bestmove " << square_to_string(best_move.from) << square_to_string(best_move.to) << std::endl;
    }
    else
    {
        // Search already completed, just join
        if (worker.joinable()) worker.join();
    }
}

void Engine::reset()
{
    should_work.store(false);
    if (worker.joinable()) worker.join();
    best_move = Move{};
    work_done.store(false);
    should_work.store(true);
}
