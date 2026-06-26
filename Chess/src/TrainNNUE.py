import struct
import random

INPUT_SIZE = 12 * 64
HIDDEN_SIZE = 128
SCALE = 256

PIECE_MAP = {
    "P": 0, "N": 1, "B": 2, "R": 3, "Q": 4, "K": 5,
    "p": 6, "n": 7, "b": 8, "r": 9, "q": 10, "k": 11,
}

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

def clamp(x, lo, hi):
    return max(lo, min(hi, x))

def forward(features, input_weights, hidden_bias, output_weights, output_bias):
    hidden = hidden_bias[:]

    for feature in features:
        base = feature * HIDDEN_SIZE
        for h in range(HIDDEN_SIZE):
            hidden[h] += input_weights[base + h]

    activated = [clamp(x, 0, 255) for x in hidden]

    score = output_bias
    for h in range(HIDDEN_SIZE):
        score += activated[h] * output_weights[h]

    return score // SCALE, hidden, activated

def save_network(path, input_weights, hidden_bias, output_weights, output_bias):
    with open(path, "wb") as f:
        f.write(b"DNNUE001")
        f.write(struct.pack("<i", INPUT_SIZE))
        f.write(struct.pack("<i", HIDDEN_SIZE))

        for x in input_weights:
            f.write(struct.pack("<h", clamp(x, -32768, 32767)))

        for x in hidden_bias:
            f.write(struct.pack("<h", clamp(x, -32768, 32767)))

        for x in output_weights:
            f.write(struct.pack("<h", clamp(x, -32768, 32767)))

        f.write(struct.pack("<i", output_bias))

def load_data(path):
    data = []

    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or "|" not in line:
                continue

            fen, score = line.rsplit("|", 1)
            target = clamp(int(score), -1000, 1000)
            data.append((features_from_fen(fen), target))

    return data

def train():
    data = load_data("lichess_big3_classical_training_data.txt")

    input_weights = [random.randint(-2, 2) for _ in range(INPUT_SIZE * HIDDEN_SIZE)]
    hidden_bias = [0 for _ in range(HIDDEN_SIZE)]
    output_weights = [random.randint(-2, 2) for _ in range(HIDDEN_SIZE)]
    output_bias = 0

    epochs = 5

    for epoch in range(epochs):
        random.shuffle(data)
        total_error = 0

        for features, target in data:
            prediction, hidden, activated = forward(
                features,
                input_weights,
                hidden_bias,
                output_weights,
                output_bias
            )

            error = target - prediction
            total_error += abs(error)

            step = clamp(error // 256, -2, 2)

            for h in range(HIDDEN_SIZE):
                if activated[h] > 0:
                    output_weights[h] += step

            output_bias += step * SCALE

        print("epoch", epoch + 1, "avg_error", total_error // max(1, len(data)))

    save_network("tiny_classical.dnnue", input_weights, hidden_bias, output_weights, output_bias)

train()