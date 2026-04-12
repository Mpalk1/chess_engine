# chess_engine

A chess engine written in C++20.


## Current status

- FEN loading is supported (`position fen ...`)
- Legal move generation for every position
- Make/unmake moves
- searching for best move (`go depth <n>`)
- UCI loop is implemented and still evolving


## Build

Requirements:

- CMake 3.28+
- C++20 compiler (GCC/Clang)
- `make` or `ninja` (any CMake-supported generator)

```bash
git clone https://github.com/Mpalk1/chess_engine.git
cd chess_engine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run:

```bash
./build/chess_engine
```

## Quick UCI usage

Minimal example session:

```text
uci
isready
position startpos
go depth 4
stop
quit
```

Commands currently handled in `src/uci.cpp` include:

- `uci`
- `isready`
- `ucinewgame`
- `position startpos [moves ...]`
- `position fen <fen> [moves ...]`
- `go depth <n>`
- `go movetime <ms>`
- `go wtime <ms> btime <ms> [winc <ms>] [binc <ms>]`
- `stop`
- `print`
- `quit`

## Testing

The repository includes an automated perft comparison script in `test/auto_perft_test.py` that checks positions against `python-chess`.

From the repository root:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r test/requirements.txt
python test/auto_perft_test.py
```

By default, the script expects the engine binary at `build/chess_engine`.

## Project layout

- `src/` - engine source code
- `test/` - perft automation and test data
- `run.sh` - quick local build/run helper

## Roadmap

- Improve UCI completeness and robustness
- Improve search quality and time management
- Expand validation and regression tests
- Tune evaluation and add stronger positional heuristics

