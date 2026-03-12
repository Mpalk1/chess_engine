#include "bitboard.h"
#include <iostream>
#include <bitset>

void Bitboard::shift(Direction dir, int times)
{
  auto d = static_cast<int>(dir);
  if (d < 0)
  {
    bitboard >>= -d * times;
  }
  else
  {
    bitboard <<= d * times;
  }
}

u64 Bitboard::get() const
{
  return bitboard;
}

void Bitboard::print_as_bits() const
{
  std::cout << std::bitset<64>(bitboard) << "\n";
}

void Bitboard::is_bit_at(const u64& x) const
{

}

void Bitboard::is_bit_at(const Square& square) const
{

}

Bitboard& Bitboard::operator<<=(int times)
{
  bitboard <<= times;
  return *this;
}

Bitboard& Bitboard::operator>>=(int times)
{
  bitboard >>= times;
  return *this;
}

Bitboard Bitboard::operator<<(int times) const
{
  auto b = bitboard << times;
  return Bitboard{b};
}

Bitboard Bitboard::operator>>(int times) const
{
  auto b = bitboard >> times;
  return Bitboard{b};
}

Bitboard& Bitboard::operator&=(u64 rhs)
{
  bitboard &= rhs;
  return *this;
}

Bitboard& Bitboard::operator|=(u64 rhs)
{
  bitboard |= rhs;
  return *this;
}

Bitboard& Bitboard::operator^=(u64 rhs)
{
  bitboard ^= rhs;
  return *this;
}

Bitboard Bitboard::operator&(u64 rhs) const
{
  return Bitboard{bitboard & rhs};
}

Bitboard Bitboard::operator|(u64 rhs) const
{
  return Bitboard{bitboard | rhs};
}

Bitboard Bitboard::operator^(u64 rhs) const
{
  return Bitboard{bitboard ^ rhs};
}

Bitboard& Bitboard::operator&=(const Bitboard& rhs)
{
  bitboard &= rhs.bitboard;
  return *this;
}

Bitboard& Bitboard::operator|=(const Bitboard& rhs)
{
  bitboard |= rhs.bitboard;
  return *this;
}

Bitboard& Bitboard::operator^=(const Bitboard& rhs)
{
  bitboard ^= rhs.bitboard;
  return *this;
}

Bitboard Bitboard::operator&(const Bitboard& rhs) const
{
  return Bitboard{bitboard & rhs.bitboard};
}

Bitboard Bitboard::operator|(const Bitboard& rhs) const
{
  return Bitboard{bitboard | rhs.bitboard};
}

Bitboard Bitboard::operator^(const Bitboard& rhs) const
{
  return Bitboard{bitboard ^ rhs.bitboard};
}

bool Bitboard::operator==(const Bitboard& rhs) const
{
  return bitboard == rhs.bitboard;
}

bool Bitboard::operator!=(const Bitboard& rhs) const
{
  return !(*this == rhs);
}

Bitboard::operator bool() const
{
  return bitboard != 0ULL;
}

Bitboard Bitboard::operator~() const
{
  return Bitboard{~bitboard};
}

