# chess_engine

A chess engine written in C++20. Currently a work in progress — the focus right now is on getting move generation **completely correct** before anything else.

## What's working

- Loads any position via FEN
- Generates all legal moves for any position (pawns, knights, bishops, rooks, queens, king — including castling, en passant, and promotions)
- Makes and unmakes moves without corrupting state
- Validates correctness via **perft** — counting leaf nodes at a given depth and comparing against known values
- Minimal UCI interface (`uci`, `isready`, `position`, `go perft <depth>`)

## What's next

- Search (minimax / alpha-beta)
- Evaluation
- Full UCI compliance so it can be plugged into a GUI

## Status

Move generation appears correct. Perft results match expected values at standard depths. The engine can't play yet — that's next.

---

## Build

**Requirements:** CMake ≥ 3.28 · C++20 compiler · Ninja · Git

### 1) Clone with submodules
```bash
git clone --recurse-submodules https://github.com/Mpalk1/chess_engine.git
cd chess_engine
```

### 2) Configure
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_SCAN_FOR_MODULES=OFF
```

### 3) Build
```bash
cmake --build build
```

### 4) Run
```bash
./build/chess_engine
```

The engine speaks UCI, so you can type commands directly or connect it to any UCI-compatible GUI (Arena, Cute Chess, etc.) - wip.
```
uci
isready
position startpos
go perft 5
```
