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
            target = max(-1000, min(1000, int(score)))
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

    return total_error / max(1, len(data))

def train():
    data = load_data("search_training_data.txt")
    random.seed(1)
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

    print("baseline validation_error", round(best_error, 3))

    # Hidden neurons 0-3 are the classical eval backbone. Train only 4-127 as a correction layer.
    for feature in range(INPUT_SIZE):
        base = feature * HIDDEN_SIZE
        for h in range(4, HIDDEN_SIZE):
            input_weights[base + h] = random.randint(-1, 1)

    for h in range(4, HIDDEN_SIZE):
        hidden_bias[h] = 0
        output_weights[h] = random.randint(-1, 1)

    epochs = 1

    for epoch in range(epochs):
        random.shuffle(train_data)
        total_error = 0
        input_updates = 0

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

            output_learning_rate = 0.00005
            input_learning_rate = 0.00000005
            bias_learning_rate = 0.0

            old_output_weights = output_weights[:]

            for h in range(4, HIDDEN_SIZE):
                if activated[h] <= 0 or activated[h] >= 255:
                    continue

                output_delta = round(output_learning_rate * error * activated[h])
                output_weights[h] += output_delta
                output_weights[h] = clamp(output_weights[h], -32768, 32767)

                hidden_error = error * old_output_weights[h] / SCALE

                raw_input_delta = input_learning_rate * hidden_error
                input_delta = round(raw_input_delta)

                if input_delta == 0:
                    continue

                input_updates += len(features)

                for feature in features:
                    index = feature * HIDDEN_SIZE + h
                    input_weights[index] += input_delta
                    input_weights[index] = clamp(input_weights[index], -32768, 32767)

            output_bias += round(bias_learning_rate * error * SCALE)
            output_bias = clamp(output_bias, -2000000, 2000000)   

        train_error = total_error / max(1, len(train_data))

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
                "train_error", round(train_error, 3),
                "validation_error", round(validation_error, 3),
                "input_updates", input_updates,
                "saved"
            )
        else:
            print(
                "epoch", epoch + 1,
                "train_error", round(train_error, 3),
                "validation_error", round(validation_error, 3),
                "input_updates", input_updates,
                "not_saved"
            )

train()
