import subprocess

ENGINE = r"..\build\Debug\dragon_engineV3.0.exe"
INPUT_FENS = "natural_fens.txt"
OUTPUT_DATA = "search_training_data.txt"
DEPTH = 3
LIMIT = 50000

def send(engine, command):
    engine.stdin.write(command + "\n")
    engine.stdin.flush()

def read_until(engine, prefix):
    while True:
        line = engine.stdout.readline()
        if not line:
            raise RuntimeError("engine stopped")

        line = line.strip()

        if line.startswith(prefix):
            return line

def main():
    engine = subprocess.Popen(
        [ENGINE],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1
    )

    send(engine, "uci")
    read_until(engine, "uciok")

    send(engine, "isready")
    read_until(engine, "readyok")

    count = 0

    with open(INPUT_FENS, "r") as fens, open(OUTPUT_DATA, "w") as out:
        for fen in fens:
            fen = fen.strip().split("|")[0]
            if not fen:
                continue

            send(engine, "ucinewgame")
            send(engine, "clearall")
            send(engine, "position fen " + fen)
            send(engine, "searcheval " + str(DEPTH))

            line = read_until(engine, "searcheval")
            score = int(line.split()[1])

            side_to_move = fen.split()[1]
            if side_to_move == "b":
                score = -score

            score = max(-1000, min(1000, score))

            out.write(f"{fen}|{score}\n")

            count += 1
            if count % 100 == 0:
                print("positions", count)

            if LIMIT and count >= LIMIT:
                break

    send(engine, "quit")
    engine.wait()

    print("wrote", count, "positions to", OUTPUT_DATA)

main()