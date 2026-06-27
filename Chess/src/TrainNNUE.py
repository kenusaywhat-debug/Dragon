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

def load_network(path):
    with open(path, "rb") as f:
        magic = f.read(8)
        if magic != b"DNNUE001":
            raise ValueError("bad NNUE magic")

        input_size = struct.unpack("<i", f.read(4))[0]
        hidden_size = struct.unpack("<i", f.read(4))[0]

        if input_size != INPUT_SIZE or hidden_size != HIDDEN_SIZE:
            raise ValueError("bad NNUE size")

        input_weights = [
            struct.unpack("<h", f.read(2))[0]
            for _ in range(INPUT_SIZE * HIDDEN_SIZE)
        ]

        hidden_bias = [
            struct.unpack("<h", f.read(2))[0]
            for _ in range(HIDDEN_SIZE)
        ]

        output_weights = [
            struct.unpack("<h", f.read(2))[0]
            for _ in range(HIDDEN_SIZE)
        ]

        output_bias = struct.unpack("<i", f.read(4))[0]

    return input_weights, hidden_bias, output_weights, output_bias

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

def average_error(data, input_weights, hidden_bias, output_weights, output_bias):
    total_error = 0

    for features, target in data:
        prediction, hidden, activated = forward(
            features,
            input_weights,
            hidden_bias,
            output_weights,
            output_bias
        )

        total_error += abs(target - prediction)

    return total_error // max(1, len(data))

def train():
    data = load_data("lichess_big3_classical_training_data.txt")
    random.shuffle(data)

    split = int(len(data) * 0.9)
    train_data = data[:split]
    validation_data = data[split:]

    input_weights, hidden_bias, output_weights, output_bias = load_network("../tiny_classical.dnnue")

    best_error = average_error(
        validation_data,
        input_weights,
        hidden_bias,
        output_weights,
        output_bias
    )

    print("baseline validation_error", best_error)

    epochs = 5

    for epoch in range(epochs):
        random.shuffle(train_data)
        total_error = 0

        for features, target in train_data:
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

        train_error = total_error // max(1, len(train_data))

        validation_error = average_error(
            validation_data,
            input_weights,
            hidden_bias,
            output_weights,
            output_bias
        )

        if validation_error < best_error:
            best_error = validation_error
            save_network("tiny_trained.dnnue", input_weights, hidden_bias, output_weights, output_bias)
            print(
                "epoch", epoch + 1,
                "train_error", train_error,
                "validation_error", validation_error,
                "saved"
            )
        else:
            print(
                "epoch", epoch + 1,
                "train_error", train_error,
                "validation_error", validation_error,
                "not_saved"
            )

train()