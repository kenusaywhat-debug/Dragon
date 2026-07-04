import chess
import chess.pgn

INPUT_PGNS = [
    r"C:\Users\kjs11\OneDrive\Desktop\Chess\DvsDnnue8.pgn",
    r"C:\Users\kjs11\OneDrive\Desktop\Chess\DvsDnnue9.pgn",
]

OUTPUT_FENS = "natural_fens.txt"

def reasonable_material(board):
    pieces = board.piece_map().values()

    white_queens = sum(1 for p in pieces if p.color == chess.WHITE and p.piece_type == chess.QUEEN)
    black_queens = sum(1 for p in pieces if p.color == chess.BLACK and p.piece_type == chess.QUEEN)

    if white_queens > 1 or black_queens > 1:
        return False

    return True

with open(OUTPUT_FENS, "w") as out:
    seen = set()

    for pgn_path in INPUT_PGNS:
        with open(pgn_path, encoding="utf-8", errors="replace") as pgn:
            while True:
                game = chess.pgn.read_game(pgn)
                if game is None:
                    break

                board = game.board()

                for ply, move in enumerate(game.mainline_moves(), start=1):
                    board.push(move)

                    if ply < 8:
                        continue
                    if ply > 80:
                        break
                    if ply % 4 != 0:
                        continue
                    if not reasonable_material(board):
                        continue

                    fen = board.fen()
                    if fen not in seen:
                        seen.add(fen)
                        out.write(fen + "\n")

print("wrote", len(seen), "fens")