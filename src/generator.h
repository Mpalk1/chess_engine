#pragma once

#include "move_list.h"
#include "types.h"
#include <array>

struct Position;

struct PositionInfo
{
    Color turn{};
    u64 empty{};
    u64 occupied{};
    u64 friendly{};
    u64 enemy{};
};

struct Generator
{
public:
    static MoveList& get_moves(Position& position, MoveList& out_moves);
    static bool is_square_attacked_by(const Position& position, Square sq, Color attacker);

    static const std::array<u64, 64> knight_attacks;
    static const std::array<u64, 64> king_attacks;
    static const std::array<std::array<u64, 64>, 8> ray_attacks;

private:
    static std::array<u64, 64> init_knight_attacks();
    static std::array<u64, 64> init_king_attacks();
    static std::array<std::array<u64, 64>, 8> init_ray_attacks();

    static void filter_legal_moves(Position& position, MoveList& moves);

    static void generate_pawn_moves(Position& position, const PositionInfo& info, MoveList& out_moves);
    static void generate_knight_moves(Position& position, const PositionInfo& info, MoveList& out_moves);
    static void generate_bishop_moves(Position& position, const PositionInfo& info, MoveList& out_moves);
    static void generate_rook_moves(Position& position, const PositionInfo& info, MoveList& out_moves);
    static void generate_queen_moves(Position& position, const PositionInfo& info, MoveList& out_moves);
    static void generate_king_moves(Position& position, const PositionInfo& info, MoveList& out_moves);
};