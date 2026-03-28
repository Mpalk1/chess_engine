import subprocess
import csv
import chess

# --- CONFIGURATION ---
PATH_TO_BINARY = '../build/chess_engine'
CSV_PATH = 'fen_testing_data.csv'
MAX_DEPTH = 4
CSV_LIMIT = 10000

# Standard stress-test positions from ChessProgramming Wiki
TEST_SUITES = [
    {"name": "Initial Position", "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
    {"name": "Kiwipete (Castling/EP)", "fen": "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"},
    {"name": "Position 3", "fen": "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
    {"name": "Position 4 (Mirrored)", "fen": "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
    {"name": "Position 5", "fen": "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"},
    {"name": "Position 6", "fen": "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"}
]

def get_engine_perft(fen, depth):
    """Communicates with your .exe via UCI-like commands."""
    try:
        process = subprocess.Popen(
            PATH_TO_BINARY,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )

        # Send commands to your engine
        commands = f"position fen {fen}\ngo perft {depth}\n"
        stdout, stderr = process.communicate(input=commands, timeout=30)

        # Parse "Nodes searched: <number>"
        for line in stdout.splitlines():
            if "Nodes searched:" in line:
                return int(line.split(":")[-1].strip())
    except Exception as e:
        return f"Error: {e}"
    return None

def get_python_chess_perft(fen, depth):
    """Calculates nodes using the trusted python-chess library."""
    board = chess.Board(fen)
    def perft(b, d):
        if d == 0: return 1
        nodes = 0
        for move in b.legal_moves:
            b.push(move)
            nodes += perft(b, d - 1)
            b.pop()
        return nodes
    return perft(board, depth)

def run_test(fen, label):
    print(f"\n--- Testing: {label} ---")
    print(f"FEN: {fen}")
    for d in range(1, MAX_DEPTH + 1):
        expected = get_python_chess_perft(fen, d)
        actual = get_engine_perft(fen, d)

        status = "PASS" if expected == actual else f"FAIL (Expected {expected}, got {actual})"
        print(f"Depth {d}: {status}")
        if expected != actual:
            break

# 1. Run Standard Test Suite
print("Starting Standard Suite...")
for test in TEST_SUITES:
    run_test(test['fen'], test['name'])

# 2. Run Lichess CSV Tests
print(f"\nStarting Lichess CSV Tests (Top {CSV_LIMIT})...")
try:
    with open(CSV_PATH, 'r') as f:
        reader = csv.DictReader(f)
        for i, row in enumerate(reader):
            if i >= CSV_LIMIT: break
            run_test(row['FEN'], f"Lichess Puzzle {row['PuzzleId']}")
except FileNotFoundError:
    print(f"CSV not found at {CSV_PATH}")
