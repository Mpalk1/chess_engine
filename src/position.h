#pragma once

#include "board_list.h"
#include "move.h"
#include "move_list.h"
#include "types.h"
#include <string>

struct Position
{
    BoardList bitboards{};
    MoveList move_list{};
    const std::string starting_fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    
    GameState state{};
    
    std::array<PieceType, 64> mailbox{};

    Position();

    u64 get_empty_squares() const { return bitboards.empty(); }
    void clear();
    void read_fen(const std::string& fen);
    void apply_move(const Move& move, GameState& prev_state);
    void undo_move(const Move& move, const GameState& prev_state);
    void make_move(const std::string& token);
    PieceType piece_at(Square sq) const;
    MoveList& get_legal_moves();
    MoveList& get_pseudo_legal_moves();
    u64 get_squares(Color color) const;
    void print() const;
    void print_moves() const { move_list.print(); }
};

bool is_number(char c);


