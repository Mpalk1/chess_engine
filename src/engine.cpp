#include "engine.h"
#include <bit>
#include <limits>

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

    return position.current_turn == Color::white ? eval/100 : -eval/100; // flipping the sign because searching is from the current players perspective
}

Move Engine::best_move(Position& position, int depth)
{
    Move best{};
    int best_val = std::numeric_limits<int>::min();

    auto moves = position.get_legal_moves();
    for (int i{0}; i < moves.count; ++i)
    {
        position.make_move(moves[i]);
        int val = -minimax(position, depth - 1);
        position.unmake_move(moves[i]);

        if (val > best_val)
        {
            best_val = val;
            best = moves[i];
        }
    }
    return best;
}

int Engine::minimax(Position& position, int depth)
{
    if (depth == 0) return evaluate(position);

    int best = std::numeric_limits<int>::min();
    auto moves = position.get_legal_moves();
    for (int i{0}; i < moves.count; ++i)
    {
        position.make_move(moves[i]);
        best = std::max(best, -minimax(position, depth - 1));
        position.unmake_move(moves[i]);
    }
    return best;
}


