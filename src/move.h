#pragma once

#include "types.h"
#include <iostream>

struct Move
{
    Square from;
    Square to;
    PieceType piece;
    PieceType captured; 
    MoveType type;
    MoveFlag flags;

    u8 castling_rights;  
    Square en_passant_square; 
    u8 halfmove_clock;   

    Move()
        : from(Square::A1), to(Square::A1), piece(PieceType::None),
          captured(PieceType::None), type(MoveType::Normal),
          flags(MoveFlag::None), castling_rights(0),
          en_passant_square(Square::None), halfmove_clock(0)
    {
    }

    Move(Square from, Square to, PieceType piece, MoveType type = MoveType::Normal)
        : from(from), to(to), piece(piece), captured(PieceType::None), type(type), flags(MoveFlag::None),
          castling_rights(0), en_passant_square(Square::None), halfmove_clock(0)
    {
    }

    Move(Square from, Square to, PieceType piece, MoveType type, PieceType captured,
         u8 castling_rights, Square en_passant_square, u8 halfmove_clock)
        : from(from), to(to), piece(piece), captured(captured), type(type), flags(MoveFlag::None),
          castling_rights(castling_rights), en_passant_square(en_passant_square), halfmove_clock(halfmove_clock)
    {
    }

    bool is_capture() const { return captured != PieceType::None; }
    bool is_promotion() const
    {
        return type == MoveType::PromotionQueen || type == MoveType::PromotionRook ||
               type == MoveType::PromotionBishop || type == MoveType::PromotionKnight ||
               type == MoveType::PromotionQueenCapture || type == MoveType::PromotionRookCapture ||
               type == MoveType::PromotionBishopCapture || type == MoveType::PromotionKnightCapture;
    }
    bool is_castle() const { return type == MoveType::KingsideCastle || type == MoveType::QueensideCastle; }
    bool is_check() const { return has_flag(flags, MoveFlag::Check); }
    bool is_checkmate() const { return has_flag(flags, MoveFlag::Checkmate); }

    void print() const
    {
        if (piece == PieceType::None)
            return;

        std::cout << square_to_string(from) << "->" << square_to_string(to) << "\n";
    }
};

inline char promotion_suffix(MoveType type)
{
    switch (type)
    {
    case MoveType::PromotionQueen:
    case MoveType::PromotionQueenCapture:
        return 'q';
    case MoveType::PromotionRook:
    case MoveType::PromotionRookCapture:
        return 'r';
    case MoveType::PromotionBishop:
    case MoveType::PromotionBishopCapture:
        return 'b';
    case MoveType::PromotionKnight:
    case MoveType::PromotionKnightCapture:
        return 'n';
    default:
        return '\0';
    }
}

inline std::string move_to_uci(const Move& move)
{
    std::string uci = square_to_string(move.from) + square_to_string(move.to);
    if (const char suffix = promotion_suffix(move.type); suffix != '\0')
        uci.push_back(suffix);
    return uci;
}
