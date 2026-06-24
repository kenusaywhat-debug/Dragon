import struct
import random

INPUT_SIZE = 12 * 64
HIDDEN_SIZE = 128

with open("tiny.dnnue", "wb") as f:
    f.write(b"DNNUE001")
    f.write(struct.pack("<i", INPUT_SIZE))
    f.write(struct.pack("<i", HIDDEN_SIZE))

    # input weights
    for _ in range(INPUT_SIZE * HIDDEN_SIZE):
        f.write(struct.pack("<h", random.randint(-2, 2)))

    # hidden bias
    for _ in range(HIDDEN_SIZE):
        f.write(struct.pack("<h", 0))

    # output weights
    for _ in range(HIDDEN_SIZE):
        f.write(struct.pack("<h", random.randint(-2, 2)))

    # output bias
    f.write(struct.pack("<i", 0))