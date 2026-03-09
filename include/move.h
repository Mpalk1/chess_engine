#pragma once
#include "move.h"
#include "types.h"

struct Move
{
  Square from; // square 0-63
  Square to; // square 0-63
  PieceType piece;
  PieceType captured; // PieceType::none if quiet move
  MoveType  type;
  MoveFlag  flags;

  u8 prev_castling_rights;   // bitmask: bit0=WK, bit1=WQ, bit2=BK, bit3=BQ
  Square prev_en_passant_sq;     // 255 = none or square index
  u8 prev_halfmove_clock;    // for 50-move rule

  Move() :
  from(Square::A1), to(Square::A1),
  piece(PieceType::none), captured(PieceType::none),
  type(MoveType::normal), flags(MoveFlag::none),
  prev_castling_rights(0), prev_en_passant_sq(Square::none), prev_halfmove_clock(0)
  {}

  Move(Square from, Square to, PieceType piece, MoveType type = MoveType::normal) :
      from(from), to(to),
      piece(piece), captured(PieceType::none),
      type(type), flags(MoveFlag::none),
      prev_castling_rights(0), prev_en_passant_sq(Square::none), prev_halfmove_clock(0)
  {}

  bool is_capture()   const { return captured != PieceType::none; }
  bool is_promotion() const
  {
      return type == MoveType::promotion_queen          ||
             type == MoveType::promotion_rook           ||
             type == MoveType::promotion_bishop         ||
             type == MoveType::promotion_knight         ||
             type == MoveType::promotion_queen_capture  ||
             type == MoveType::promotion_rook_capture   ||
             type == MoveType::promotion_bishop_capture ||
             type == MoveType::promotion_knight_capture;
  }
  bool is_castle() const { return type == MoveType::kingside_castle || type == MoveType::queenside_castle;}
  bool is_check()     const { return has_flag(flags, MoveFlag::check); }
  bool is_checkmate() const { return has_flag(flags, MoveFlag::checkmate); }
};

