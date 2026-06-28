import struct
import sys

INPUT_SIZE = 12 * 64
HIDDEN_SIZE = 128
SCALE = 256

PIECE_MAP = {
    "P": 0, "N": 1, "B": 2, "R": 3, "Q": 4, "K": 5,
    "p": 6, "n": 7, "b": 8, "r": 9, "q": 10, "k": 11,
}

def clamp(x, lo, hi):
    return max(lo, min(hi, x))

def features_from_fen(fen):
    board = fen.split()[0]
    features = []
    square = 0

    for char in board:
        if char == "/":
            continue
        if char.isdigit():
            square += int(char)
            continue

        piece = PIECE_MAP[char]
        features.append(piece * 64 + square)
        square += 1

    return features

def load_network(path):
    with open(path, "rb") as f:
        magic = f.read(8)
        if magic != b"DNNUE001":
            raise ValueError("bad NNUE magic")

        input_size = struct.unpack("<i", f.read(4))[0]
        hidden_size = struct.unpack("<i", f.read(4))[0]

        if input_size != INPUT_SIZE or hidden_size != HIDDEN_SIZE:
            raise ValueError("bad NNUE size")

        input_weights = [struct.unpack("<h", f.read(2))[0] for _ in range(INPUT_SIZE * HIDDEN_SIZE)]
        hidden_bias = [struct.unpack("<h", f.read(2))[0] for _ in range(HIDDEN_SIZE)]
        output_weights = [struct.unpack("<h", f.read(2))[0] for _ in range(HIDDEN_SIZE)]
        output_bias = struct.unpack("<i", f.read(4))[0]

    return input_weights, hidden_bias, output_weights, output_bias

def forward(features, input_weights, hidden_bias, output_weights, output_bias):
    hidden = hidden_bias[:]

    for feature in features:
        base = feature * HIDDEN_SIZE
        for h in range(HIDDEN_SIZE):
            hidden[h] += input_weights[base + h]

    score = output_bias

    for h in range(HIDDEN_SIZE):
        activated = clamp(hidden[h], 0, 255)
        score += activated * output_weights[h]

    return score // SCALE

def validate(network_path, data_path, limit):
    net = load_network(network_path)

    count = 0
    total_abs_error = 0
    total_signed_error = 0
    max_error = 0
    worst = None

    with open(data_path, "r") as f:
        for line in f:
            if limit and count >= limit:
                break

            line = line.strip()
            if not line or "|" not in line:
                continue

            fen, target_text = line.rsplit("|", 1)
            target = clamp(int(target_text), -1000, 1000)
            prediction = forward(features_from_fen(fen), *net)

            error = target - prediction
            abs_error = abs(error)

            total_abs_error += abs_error
            total_signed_error += error
            count += 1

            if abs_error > max_error:
                max_error = abs_error
                worst = (fen, target, prediction, error)

    print("positions", count)
    print("avg_abs_error", round(total_abs_error / max(1, count), 3))
    print("avg_signed_error", round(total_signed_error / max(1, count), 3))
    print("max_error", max_error)

    if worst:
        fen, target, prediction, error = worst
        print("worst_target", target)
        print("worst_prediction", prediction)
        print("worst_error", error)
        print("worst_fen", fen)

if __name__ == "__main__":
    network_path = sys.argv[1]
    data_path = sys.argv[2]
    limit = int(sys.argv[3]) if len(sys.argv) > 3 else 10000

    validate(network_path, data_path, limit)