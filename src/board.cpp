#include "board.h"
#include <bit>
#include <cassert>
#include <cctype>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include "bitboard.h"
#include "move_list.h"
#include "types.h"

Board::Board()
{
  init_knight_attacks();
  init_king_attacks();
  init_ray_attacks();
}

void Board::clear()
{
  bitboards.clear();
  move_list.clear();
  current_turn = Color::white;
  castling_rights = 0;
  en_passant_square = Square::none;
  halfmove_clock = 0;
  fullmove_number = 1;
  previous_move = Move{};
}

void Board::make_move(Square from, Square to)
{
  const u64 from_bb = 1ULL << static_cast<int>(from);
  const u64 to_bb = 1ULL << static_cast<int>(to);

  // Find which piece is on the from square
  PieceType moving_piece = PieceType::none;
  for (int i = 0; i < 12; ++i)
  {
    if (bitboards[i] & from_bb)
    {
      moving_piece = static_cast<PieceType>(i);
      break;
    }
  }

  if (moving_piece == PieceType::none)
    return;

  // Clear any captured piece on the destination square
  for (int i = 0; i < 12; ++i)
  {
    if (bitboards[i] & to_bb)
    {
      bitboards[static_cast<PieceType>(i)] &= ~to_bb;
      break;
    }
  }

  // Move the piece: clear from, set to
  bitboards[moving_piece] &= ~from_bb;
  bitboards[moving_piece] |= to_bb;

  current_turn = (current_turn == Color::white) ? Color::black : Color::white;
}

void Board::make_move(Move &move) {}

void Board::unmake_move(Move &move) {}

void Board::unmake_move(Square from, Square to)
{
  
}

MoveList Board::get_legal_moves() { return MoveList{}; }

const MoveList& Board::get_pseudo_legal_moves()
{
  move_list.clear();
  generate_pawn_moves();
  generate_knight_moves();
  generate_bishop_moves();
  generate_king_moves();
  generate_queen_moves();
  generate_rook_moves();
  return move_list;
}

void Board::generate_pawn_moves()
{
  const auto empty_squares = get_empty_squares();
  const auto enemy_pieces = (current_turn == Color::white) ? get_squares(Color::black) : get_squares(Color::white);

  auto pawns =
      (current_turn == Color::white) ? bitboards[PieceType::white_pawn].get() : bitboards[PieceType::black_pawn].get();

  const auto dir = (current_turn == Color::white) ? Direction::north : Direction::south;
  const auto cap_left = (current_turn == Color::white) ? Direction::north_west : Direction::south_east;
  const auto cap_right = (current_turn == Color::white) ? Direction::north_east : Direction::south_west;
  const auto piece_type = (current_turn == Color::white) ? PieceType::white_pawn : PieceType::black_pawn;
  const auto start_rank = (current_turn == Color::white) ? RANK_2 : RANK_7;

  while (pawns)
  {
    const auto from = std::countr_zero(pawns);
    const Bitboard from_bb{1ULL << from};

    // --- quiet moves (unchanged) ---
    Bitboard single_push_bb = from_bb.shift(dir, 1) & empty_squares;
    u64 s = single_push_bb.get();
    while (s)
    {
      const auto to = std::countr_zero(s);
      move_list.add(make_square(from), make_square(to), piece_type, MoveType::normal,
                    PieceType::none, castling_rights, en_passant_square, halfmove_clock);
      s &= s - 1;
    }

    if (from_bb.get() & start_rank)
    {
      // only allow double push if the square in between is empty
      Bitboard double_push_bb = from_bb.shift(dir, 2) & empty_squares & (single_push_bb.shift(dir, 1));
      u64 d = double_push_bb.get();
      while (d)
      {
        const auto to = std::countr_zero(d);
        move_list.add(make_square(from), make_square(to), piece_type, MoveType::double_pawn_push,
                      PieceType::none, castling_rights, en_passant_square, halfmove_clock);
        d &= d - 1;
      }
    }

    // --- captures ---
    const Bitboard attacks = (from_bb.shift(cap_left, 1) | from_bb.shift(cap_right, 1)) & enemy_pieces;
    u64 a = attacks.get();
    while (a)
    {
      const auto to = std::countr_zero(a);
      move_list.add(make_square(from), make_square(to), piece_type, MoveType::capture,
                    bitboards.piece_at(make_square(to)), castling_rights, en_passant_square, halfmove_clock);
      a &= a - 1;
    }

    pawns &= pawns - 1;
  }
}
void Board::generate_knight_moves()
{
  const auto friendly_squares = get_squares(current_turn == Color::white ? Color::white : Color::black);
  auto knights = bitboards[current_turn == Color::white ? PieceType::white_knight : PieceType::black_knight].get();
  while (knights)
  {
    auto from_idx = std::countr_zero(knights);
    const auto from = make_square(from_idx);

    const u64 moves = knight_attacks[from_idx] & ~friendly_squares;

    move_list.add_moves(from, moves,
                         current_turn == Color::white ? PieceType::white_knight : PieceType::black_knight,
                         bitboards, castling_rights, en_passant_square, halfmove_clock);

    knights &= knights - 1; // clear the first bit set to 1 counting from LSB
  }
}
void Board::generate_bishop_moves()
{
  const auto piece_type = current_turn == Color::white ? PieceType::white_bishop : PieceType::black_bishop;
  auto bishops = bitboards[piece_type].get();
  const u64 occupied = ~get_empty_squares();
  const u64 friendly = get_squares(current_turn == Color::white ? Color::white : Color::black);

  while (bishops)
  {
    const int from_idx = std::countr_zero(bishops);

    u64 attacks = 0;

    for (const auto& dir : {1, 7})
    {
      const u64 ray = ray_attacks[dir][from_idx];
      const u64 blockers = ray & occupied;
      if (blockers)
      {
        const int first_blocker = std::countr_zero(blockers);
        attacks |= ray ^ ray_attacks[dir][first_blocker];
      }
      else
        attacks |= ray;
    }

    for (const auto& dir : {3, 5})
    {
      const u64 ray = ray_attacks[dir][from_idx];
      const u64 blockers = ray & occupied;
      if (blockers)
      {
        const int first_blocker = 63 - std::countl_zero(blockers);
        attacks |= ray ^ ray_attacks[dir][first_blocker];
      }
      else
        attacks |= ray;
    }

    attacks &= ~friendly;
    move_list.add_moves(make_square(from_idx), attacks, piece_type, bitboards, castling_rights, en_passant_square, halfmove_clock);

    bishops &= bishops - 1;
  }
}
void Board::generate_rook_moves()
{
  const auto piece_type = current_turn == Color::white ? PieceType::white_rook : PieceType::black_rook;
  auto rooks = bitboards[piece_type].get();
  const u64 occupied = ~get_empty_squares();
  const u64 friendly = get_squares(current_turn == Color::white ? Color::white : Color::black);

  while (rooks)
  {
    const int from_idx = std::countr_zero(rooks);

    u64 attacks = 0;

    for (int dir : {0, 2})
    {
      const u64 ray = ray_attacks[dir][from_idx];
      const u64 blockers = ray & occupied;
      if (blockers)
      {
        const int first_blocker = std::countr_zero(blockers);
        attacks |= ray ^ ray_attacks[dir][first_blocker];
      }
      else
        attacks |= ray;
    }

    for (int dir : {4, 6})
    {
      const u64 ray = ray_attacks[dir][from_idx];
      const u64 blockers = ray & occupied;
      if (blockers)
      {
        const int first_blocker = 63 - std::countl_zero(blockers);
        attacks |= ray ^ ray_attacks[dir][first_blocker];
      }
      else
        attacks |= ray;
    }

    attacks &= ~friendly;
    move_list.add_moves(make_square(from_idx), attacks, piece_type, bitboards, castling_rights, en_passant_square, halfmove_clock);

    rooks &= rooks - 1;
  }
}
void Board::generate_queen_moves()
{
  const auto piece_type = current_turn == Color::white ? PieceType::white_queen : PieceType::black_queen;
  auto queens = bitboards[piece_type].get();
  const u64 occupied = ~get_empty_squares();
  const u64 friendly = get_squares(current_turn == Color::white ? Color::white : Color::black);

  while (queens)
  {
    const int from_idx = std::countr_zero(queens);

    u64 attacks = 0;

    for (int dir : {0, 1, 2, 7})
    {
      const u64 ray = ray_attacks[dir][from_idx];
      const u64 blockers = ray & occupied;
      if (blockers)
      {
        const int first_blocker = std::countr_zero(blockers);
        attacks |= ray ^ ray_attacks[dir][first_blocker];
      }
      else
        attacks |= ray;
    }

    for (int dir : {3, 4, 5, 6})
    {
      const u64 ray = ray_attacks[dir][from_idx];
      const u64 blockers = ray & occupied;
      if (blockers)
      {
        const int first_blocker = 63 - std::countl_zero(blockers);
        attacks |= ray ^ ray_attacks[dir][first_blocker];
      }
      else
        attacks |= ray;
    }

    attacks &= ~friendly;
    move_list.add_moves(make_square(from_idx), attacks, piece_type, bitboards, castling_rights, en_passant_square, halfmove_clock);
    queens &= queens - 1;
  }
}
void Board::generate_king_moves()
{
  const auto friendly_squares = get_squares(current_turn == Color::white ? Color::white : Color::black);
  const auto piece_type = current_turn == Color::white ? PieceType::white_king : PieceType::black_king;
  auto king = bitboards[piece_type].get();
  const auto from = std::countr_zero(king);
  auto moves = king_attacks[from] & ~friendly_squares;
  while (moves)
  {
    const auto move_idx = std::countr_zero(moves);
    move_list.add(make_square(from), make_square(move_idx), piece_type, MoveType::normal,
                  bitboards.piece_at(make_square(move_idx)), castling_rights, en_passant_square, halfmove_clock);
    moves &= moves - 1;
  }
}

void Board::print() const
{
  constexpr char pieces[] = "PNBRQKpnbrqk";

  for (int rank = 7; rank >= 0; rank--)
  {
    std::cout << rank + 1 << "  ";
    for (int file = 0; file < 8; file++)
    {
      const int square = rank * 8 + file;
      char found = '.';

      for (int i = 0; i < 12; i++)
      {
        if (bitboards[i] & (1ULL << square))
        {
          found = pieces[i];
          break;
        }
      }
      std::cout << found << ' ';
    }
    std::cout << '\n';
  }
  std::cout << "   A B C D E F G H\n\n";

  // for (int i = 0; i < 12; i++)
  //   std::cout << "bitboard[" << i << "] = "
  //             << std::bitset<64>(bitboards[i]) << '\n';
}

u64 Board::get_squares(Color color) const { return bitboards.occupied(color); }

constexpr void Board::init_knight_attacks()
{
  for (int s = 0; s < 64; ++s)
  {
    u64 b = 1ULL << s;
    u64 attacks = 0;

    attacks |= (b << 17) & ~FILE_A;
    attacks |= (b << 15) & ~FILE_H;
    attacks |= (b >> 17) & ~FILE_H;
    attacks |= (b >> 15) & ~FILE_A;

    attacks |= (b << 10) & ~(FILE_A | FILE_B);
    attacks |= (b << 6) & ~(FILE_G | FILE_H);
    attacks |= (b >> 10) & ~(FILE_G | FILE_H);
    attacks |= (b >> 6) & ~(FILE_A | FILE_B);

    knight_attacks[s] = attacks;
  }
}

constexpr void Board::init_king_attacks()
{
  for (int s = 0; s < 64; ++s)
  {
    u64 b = 1ULL << s;
    u64 attacks = 0;

    // vertical
    attacks |= (b << 8);
    attacks |= (b >> 8);

    // horizontal
    attacks |= (b << 1) & ~FILE_A;
    attacks |= (b >> 1) & ~FILE_H;

    // diagonals
    attacks |= (b << 9) & ~FILE_A;
    attacks |= (b << 7) & ~FILE_H;
    attacks |= (b >> 9) & ~FILE_H;
    attacks |= (b >> 7) & ~FILE_A;

    king_attacks[s] = attacks;
  }
}

constexpr void Board::init_ray_attacks()
{
  for (int s = 0; s < 64; ++s)
  {
    int rank = s / 8;
    int file = s % 8;

    // 0 - North (+8)
    u64 ray = 0;
    for (int r = rank + 1; r < 8; ++r)
      ray |= 1ULL << (r * 8 + file);
    ray_attacks[0][s] = ray;

    // 1 - North East (+9)
    ray = 0;
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f)
      ray |= 1ULL << (r * 8 + f);
    ray_attacks[1][s] = ray;

    // 2 - East (+1)
    ray = 0;
    for (int f = file + 1; f < 8; ++f)
      ray |= 1ULL << (rank * 8 + f);
    ray_attacks[2][s] = ray;

    // 3 - South East (-7)
    ray = 0;
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f)
      ray |= 1ULL << (r * 8 + f);
    ray_attacks[3][s] = ray;

    // 4 - South (-8)
    ray = 0;
    for (int r = rank - 1; r >= 0; --r)
      ray |= 1ULL << (r * 8 + file);
    ray_attacks[4][s] = ray;

    // 5 - South West (-9)
    ray = 0;
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f)
      ray |= 1ULL << (r * 8 + f);
    ray_attacks[5][s] = ray;

    // 6 - West (-1)
    ray = 0;
    for (int f = file - 1; f >= 0; --f)
      ray |= 1ULL << (rank * 8 + f);
    ray_attacks[6][s] = ray;

    // 7 - North West (+7)
    ray = 0;
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f)
      ray |= 1ULL << (r * 8 + f);
    ray_attacks[7][s] = ray;
  }
}

bool is_number(char c) { return std::isdigit(static_cast<unsigned char>(c)); }

void Board::read_fen(const std::string &fen)
{
  assert(!fen.empty());
  clear();

  auto split_fen = [](std::string_view fen)
  {
    std::array<std::string_view, 6> fields{};

    size_t start = 0;
    size_t end;
    int i = 0;

    while ((end = fen.find(' ', start)) != std::string_view::npos && i < 5)
    {
      fields[i++] = fen.substr(start, end - start);
      start = end + 1;
    }

    fields[i] = fen.substr(start);

    return fields;
  };

  auto parse_square = [](const std::string_view sq) -> Square
  {
    if (sq.size() != 2)
      return Square::none;

    const char file_char = static_cast<char>(std::tolower(static_cast<unsigned char>(sq[0])));
    const char rank_char = sq[1];
    if (file_char < 'a' || file_char > 'h' || rank_char < '1' || rank_char > '8')
      return Square::none;

    const int file_idx = file_char - 'a';
    const int rank_idx = rank_char - '1';
    return make_square(file_idx, rank_idx);
  };

  const auto fields = split_fen(fen);

  static const std::unordered_map<char, PieceType> charToPiece = {
      {'P', PieceType::white_pawn}, {'N', PieceType::white_knight}, {'B', PieceType::white_bishop},
      {'R', PieceType::white_rook}, {'Q', PieceType::white_queen},  {'K', PieceType::white_king},
      {'p', PieceType::black_pawn}, {'n', PieceType::black_knight}, {'b', PieceType::black_bishop},
      {'r', PieceType::black_rook}, {'q', PieceType::black_queen},  {'k', PieceType::black_king},
  };

  int ctr = 0;
  for (const char c: fields[0])
  {
    if (c == '/')
      continue;

    if (is_number(c))
    {
      ctr += c - '0';
      continue;
    }

    if (auto it = charToPiece.find(c); it != charToPiece.end())
    {
      const int rank = 7 - (ctr / 8);
      const int file = ctr % 8;
      const int square = rank * 8 + file;
      bitboards[it->second] |= (1ULL << square);
      ctr++;
    }
  }

  if (fields[1] == "w")
    current_turn = Color::white;
  else if (fields[1] == "b")
    current_turn = Color::black;

  if (fields[2] != "-")
  {
    for (const char& c: fields[2])
    {
      if (c == 'K')
        castling_rights |= castle_white_kingside;
      else if (c == 'Q')
        castling_rights |= castle_white_queenside;
      else if (c == 'k')
        castling_rights |= castle_black_kingside;
      else if (c == 'q')
        castling_rights |= castle_black_queenside;
    }
  }

  en_passant_square = (fields[3] == "-") ? Square::none : parse_square(fields[3]);

  if (!fields[4].empty())
    halfmove_clock = static_cast<u8>(std::stoi(std::string(fields[4])));

  if (!fields[5].empty())
    fullmove_number = std::stoi(std::string(fields[5]));
}
