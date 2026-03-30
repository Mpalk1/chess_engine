#include "board_list.h"

Bitboard& BoardList::operator[](PieceType piece)
{
	return bitboards[piece_val(piece)];
}

const Bitboard& BoardList::operator[](PieceType piece) const
{
	return bitboards[piece_val(piece)];
}

const Bitboard& BoardList::operator[](int idx) const
{
	return bitboards[idx];
}

void BoardList::clear()
{
	for (auto& bb : bitboards)
	{
		bb = Bitboard{ 0ULL };
	}
}

u64 BoardList::occupied() const
{
	u64 occ = 0ULL;
	for (const auto& bb : bitboards)
	{
		occ |= bb.get();
	}
	return occ;
}

u64 BoardList::occupied(Color color) const
{
	if (color == Color::none)
	{
		return 0ULL;
	}

	u64 occ = 0ULL;
	const int begin = (color == Color::white) ? 0 : 6;
	const int end = (color == Color::white) ? 6 : 12;
	for (int i = begin; i < end; ++i)
	{
		occ |= bitboards[i].get();
	}
	return occ;
}

u64 BoardList::empty() const
{
	return ~occupied();
}
// todo: make this use a mailbox approach -> tracking which piece is where when making moves
PieceType BoardList::piece_at(Square s) const
{
	if (s == Square::none)
		return PieceType::none;
	const u64 mask = 1ULL << static_cast<u8>(s);
	for (int i = 0; i < 12; ++i)
	{
		if (bitboards[i].get() & mask)
			return static_cast<PieceType>(i);
	}
	return PieceType::none;
}

