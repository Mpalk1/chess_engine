#include "position.h"
#include "bitboard.h"
#include "generator.h"
#include "move_list.h"
#include "types.h"
#include "zobrist.h"
#include <cassert>
#include <cctype>
#include <iostream>
#include <string_view>
#include <unordered_map>

namespace
{
PieceType promoted_piece_for_move(MoveType type, Color side)
{
    const bool is_white = (side == Color::White);
    switch (type)
    {
    case MoveType::PromotionQueen:
    case MoveType::PromotionQueenCapture:
        return is_white ? PieceType::WhiteQueen : PieceType::BlackQueen;
    case MoveType::PromotionRook:
    case MoveType::PromotionRookCapture:
        return is_white ? PieceType::WhiteRook : PieceType::BlackRook;
    case MoveType::PromotionBishop:
    case MoveType::PromotionBishopCapture:
        return is_white ? PieceType::WhiteBishop : PieceType::BlackBishop;
    default:
        return is_white ? PieceType::WhiteKnight : PieceType::BlackKnight;
    }
}

void clear_square(Position& pos, Square sq, PieceType piece)
{
    if (sq == Square::None || piece == PieceType::None)
        return;

    const int idx = static_cast<int>(sq);
    pos.mailbox[idx] = PieceType::None;
    pos.bitboards[piece] &= ~(1ULL << idx);
}

void set_square(Position& pos, Square sq, PieceType piece)
{
    if (sq == Square::None || piece == PieceType::None)
        return;

    const int idx = static_cast<int>(sq);
    pos.mailbox[idx] = piece;
    pos.bitboards[piece] |= (1ULL << idx);
}
} 

Position::Position() = default;

void Position::clear()
{
    bitboards.clear();
    mailbox.fill(PieceType::None);
    move_list.clear();
    state = GameState{};
}

void Position::apply_move(const Move& move, GameState& prev_state)
{
    prev_state = state;

    if (move.piece == PieceType::None)
        return;

    if (move.type == MoveType::KingsideCastle)
    {
        clear_square(*this, move.from, move.piece);
        set_square(*this, move.to, move.piece);

        if (state.turn == Color::White)
        {
            clear_square(*this, Square::H1, PieceType::WhiteRook);
            set_square(*this, Square::F1, PieceType::WhiteRook);
        }
        else
        {
            clear_square(*this, Square::H8, PieceType::BlackRook);
            set_square(*this, Square::F8, PieceType::BlackRook);
        }
    }
    else if (move.type == MoveType::QueensideCastle)
    {
        clear_square(*this, move.from, move.piece);
        set_square(*this, move.to, move.piece);

        if (state.turn == Color::White)
        {
            clear_square(*this, Square::A1, PieceType::WhiteRook);
            set_square(*this, Square::D1, PieceType::WhiteRook);
        }
        else
        {
            clear_square(*this, Square::A8, PieceType::BlackRook);
            set_square(*this, Square::D8, PieceType::BlackRook);
        }
    }
    else if (move.type == MoveType::EnPassant)
    {
        clear_square(*this, move.from, move.piece);
        set_square(*this, move.to, move.piece);

        const int captured_sq = (state.turn == Color::White) ? static_cast<int>(move.to) - 8 : static_cast<int>(move.to) + 8;
        clear_square(*this, make_square(captured_sq), move.captured);
    }
    else if (move.is_promotion())
    {
        const PieceType promo_piece = promoted_piece_for_move(move.type, state.turn);

        clear_square(*this, move.from, move.piece);
        if (move.captured != PieceType::None)
            clear_square(*this, move.to, move.captured);
        set_square(*this, move.to, promo_piece);
    }
    else
    {
        clear_square(*this, move.from, move.piece);
        if (move.captured != PieceType::None)
            clear_square(*this, move.to, move.captured);
        set_square(*this, move.to, move.piece);
    }

    state.castling_rights = move.castling_rights;
    state.en_passant_square = move.en_passant_square;
    state.halfmove_clock = move.halfmove_clock;
    if (state.turn == Color::Black)
        ++state.fullmove_number;
    state.turn = (state.turn == Color::White) ? Color::Black : Color::White;
    state.zobrist_key = Zobrist::calculate_hash(*this);
}

void Position::undo_move(const Move& move, const GameState& prev_state)
{
    if (move.piece == PieceType::None)
        return;

    if (move.type == MoveType::KingsideCastle)
    {
        clear_square(*this, move.to, move.piece);
        set_square(*this, move.from, move.piece);

        if (prev_state.turn == Color::White)
        {
            clear_square(*this, Square::F1, PieceType::WhiteRook);
            set_square(*this, Square::H1, PieceType::WhiteRook);
        }
        else
        {
            clear_square(*this, Square::F8, PieceType::BlackRook);
            set_square(*this, Square::H8, PieceType::BlackRook);
        }
    }
    else if (move.type == MoveType::QueensideCastle)
    {
        clear_square(*this, move.to, move.piece);
        set_square(*this, move.from, move.piece);

        if (prev_state.turn == Color::White)
        {
            clear_square(*this, Square::D1, PieceType::WhiteRook);
            set_square(*this, Square::A1, PieceType::WhiteRook);
        }
        else
        {
            clear_square(*this, Square::D8, PieceType::BlackRook);
            set_square(*this, Square::A8, PieceType::BlackRook);
        }
    }
    else if (move.type == MoveType::EnPassant)
    {
        clear_square(*this, move.to, move.piece);
        set_square(*this, move.from, move.piece);

        const int captured_sq = (prev_state.turn == Color::White) ? static_cast<int>(move.to) - 8 : static_cast<int>(move.to) + 8;
        set_square(*this, make_square(captured_sq), move.captured);
    }
    else if (move.is_promotion())
    {
        const PieceType promo_piece = promoted_piece_for_move(move.type, prev_state.turn);

        clear_square(*this, move.to, promo_piece);
        set_square(*this, move.from, move.piece);

        if (move.captured != PieceType::None)
            set_square(*this, move.to, move.captured);
    }
    else
    {
        clear_square(*this, move.to, move.piece);
        set_square(*this, move.from, move.piece);

        if (move.captured != PieceType::None)
            set_square(*this, move.to, move.captured);
    }

    state = prev_state;
}

MoveList& Position::get_legal_moves()
{
    return Generator::get_moves(*this, move_list);
}

MoveList& Position::get_pseudo_legal_moves()
{
    return Generator::get_moves(*this, move_list);
}

void Position::print() const
{
    constexpr char pieces[] = "PNBRQKpnbrqk";

    for (int rank = 7; rank >= 0; rank--)
    {
        std::cout << rank + 1 << "  ";
        for (int file = 0; file < 8; file++)
        {
            const int square = rank * 8 + file;
            const PieceType piece = mailbox[square];
            char found = (piece == PieceType::None) ? '.' : pieces[piece_val(piece)];
            std::cout << found << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "   A B C D E F G H\n\n";
}

u64 Position::get_squares(Color color) const
{
    return bitboards.occupied(color);
}

bool is_number(char c)
{
    return std::isdigit(static_cast<unsigned char>(c));
}

void Position::read_fen(const std::string& fen)
{
    assert(!fen.empty());
    clear();

    auto split_fen = [](std::string_view fen) {
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

    const auto fields = split_fen(fen);

    static const std::unordered_map<char, PieceType> charToPiece = {
        {'P', PieceType::WhitePawn},   {'N', PieceType::WhiteKnight},  {'B', PieceType::WhiteBishop},
        {'R', PieceType::WhiteRook},   {'Q', PieceType::WhiteQueen},   {'K', PieceType::WhiteKing},
        {'p', PieceType::BlackPawn},   {'n', PieceType::BlackKnight},  {'b', PieceType::BlackBishop},
        {'r', PieceType::BlackRook},   {'q', PieceType::BlackQueen},   {'k', PieceType::BlackKing},
    };

    int ctr = 0;
    for (const char c : fields[0])
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
            const auto idx = 1ULL << square;
            bitboards[it->second] |= (idx);
            mailbox[square] = it->second;
            ctr++;
        }
    }

    if (fields[1] == "w")
        state.turn = Color::White;
    else if (fields[1] == "b")
        state.turn = Color::Black;

    if (fields[2] != "-")
    {
        for (const char& c : fields[2])
        {
            if (c == 'K')
                state.castling_rights |= CASTLE_WHITE_KINGSIDE;
            else if (c == 'Q')
                state.castling_rights |= CASTLE_WHITE_QUEENSIDE;
            else if (c == 'k')
                state.castling_rights |= CASTLE_BLACK_KINGSIDE;
            else if (c == 'q')
                state.castling_rights |= CASTLE_BLACK_QUEENSIDE;
        }
    }

    state.en_passant_square = (fields[3] == "-") ? Square::None : square_from_string(fields[3]);

    if (!fields[4].empty())
        state.halfmove_clock = static_cast<u8>(std::stoi(std::string(fields[4])));

    if (!fields[5].empty())
        state.fullmove_number = std::stoi(std::string(fields[5]));

    state.zobrist_key = Zobrist::calculate_hash(*this);
}

PieceType Position::piece_at(Square sq) const
{
    if (sq == Square::None)
        return PieceType::None;
    return mailbox[static_cast<int>(sq)];
}

void Position::make_move(const std::string& token)
{
    if (token.size() != 4 && token.size() != 5)
        return;

    const Square from = square_from_chars(token[0], token[1]);
    const Square to = square_from_chars(token[2], token[3]);
    if (from == Square::None || to == Square::None)
        return;

    char promotion_piece = '\0';
    if (token.size() == 5)
    {
        promotion_piece = static_cast<char>(std::tolower(static_cast<unsigned char>(token[4])));
        if (promotion_piece != 'q' && promotion_piece != 'r' && promotion_piece != 'b' && promotion_piece != 'n')
            return;
    }

    auto promotion_matches = [](MoveType type, char promo) -> bool {
        switch (promo)
        {
        case 'q':
            return type == MoveType::PromotionQueen || type == MoveType::PromotionQueenCapture;
        case 'r':
            return type == MoveType::PromotionRook || type == MoveType::PromotionRookCapture;
        case 'b':
            return type == MoveType::PromotionBishop || type == MoveType::PromotionBishopCapture;
        case 'n':
            return type == MoveType::PromotionKnight || type == MoveType::PromotionKnightCapture;
        default:
            return false;
        }
    };

    MoveList& legal_moves = get_legal_moves();
    for (int i = 0; i < static_cast<int>(legal_moves.count); ++i)
    {
        const Move& move = legal_moves[i];
        if (move.from != from || move.to != to)
            continue;

        if (token.size() == 5)
        {
            if (!move.is_promotion() || !promotion_matches(move.type, promotion_piece))
                continue;
        }
        else if (move.is_promotion())
        {
            continue;
        }

        GameState dummy_state{};
        apply_move(move, dummy_state);
        return;
    }
}
