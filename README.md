# chess_engine

A small bitboard-based chess engine written in modern C++ (`C++20`), currently focused on **move generation correctness** and **perft-style validation** through a minimal UCI interface.

> Status: **work in progress**

## What is currently in the project

### Core architecture
- Bitboard-based board representation (`12` piece bitboards: white/black × 6 piece types).
- Board state tracking:
  - side to move
  - castling rights
  - en passant square
  - halfmove/fullmove counters

### Position setup
- FEN parsing via `Board::read_fen(...)`.
- Built-in starting position FEN.

### Move system
- `Move` + `MoveList` types with support for:
  - normal moves
  - captures
  - double pawn push
  - en passant
  - castling (king/queen side)
  - all promotion variants (with and without capture)

### Move generation
- Pseudo-legal generation for:
  - pawns
  - knights
  - bishops
  - rooks
  - queens
  - king
- Legal move filtering (removes moves that leave own king in check).
- Precomputed attack tables:
  - knight attacks
  - king attacks
  - directional ray attacks for sliders

### Move execution
- `make_move(...)` / `unmake_move(...)` for all supported move types, including special moves.
- Used by recursive `perft` for correctness testing.

### UCI (currently minimal)
Implemented commands:
- `uci`
- `isready`
- `position startpos`
- `go perft <depth>`

Current focus is validation/debugging rather than playing strength.

### Profiling
- Integrated with Tracy (`third_party/tracy`) and instrumented with `ZoneScoped` in hot paths.

---

## Build

## Requirements
- CMake `>= 3.28`
- C++20 compiler (MSVC is fine)
- Ninja
- Git (for submodules)

## 1) Clone (with submodules)
