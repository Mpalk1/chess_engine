#include "engine.h"
#include <bit>
#include <limits>

Engine::Engine()
{
    thread_count = std::max(1u, std::thread::hardware_concurrency());
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

    auto moves = position.get_legal_moves();
    for (int i{0}; i < moves.count; ++i)
    {
        if (!should_work.load()) break;
        position.make_move(moves[i]);
        int val = -minimax(position, depth - 1, -beta, -alpha);
        position.unmake_move(moves[i]);

        if (val > best_val)
        {
            best_val = val;
            best_move = moves[i];
        }

std::cout << "Move: " << square_to_string(moves[i].from) << square_to_string(moves[i].to)
          << " Value: " << val << std::endl;


        alpha = std::max(alpha, best_val);
        if (alpha >= beta) break;

    }
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

int Engine::minimax(Position& position, int depth, int alpha, int beta)
{
    if (depth == 0) return evaluate(position);

    int best = std::numeric_limits<int>::min();
    auto moves = position.get_legal_moves();
    for (int i{0}; i < moves.count; ++i)
    {
        if (!should_work.load()) break;
        position.make_move(moves[i]);
        best = std::max(best, -minimax(position, depth - 1, -beta, -alpha));
        position.unmake_move(moves[i]);

        alpha = std::max(alpha, best);
        if (alpha >= beta) break;
    }
    return best;
}

void Engine::stop()
{
    should_work.store(false);
    if (work_done.load()) std::cout << "bestmove " << square_to_string(best_move.from) << square_to_string(best_move.to) << std::endl;
    if (worker.joinable()) worker.join();
}

void Engine::reset()
{
    should_work.store(false);
    if (worker.joinable()) worker.join();
    best_move = Move{};
    work_done.store(false);
    should_work.store(true);
}
