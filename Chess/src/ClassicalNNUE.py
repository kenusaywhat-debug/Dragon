import struct

INPUT_SIZE = 12 * 64
HIDDEN_SIZE = 128
SCALE = 256
FEATURE_SCALE = 16

PIECE_VALUES = {
    0: 100, 1: 300, 2: 300, 3: 500, 4: 900, 5: 0,
    6: 100, 7: 300, 8: 300, 9: 500, 10: 900, 11: 0,
}

pawn = [0,0,0,0,0,0,0,0,50,50,50,50,50,50,50,50,10,10,20,30,30,20,10,10,5,5,10,25,25,10,5,5,0,0,0,20,20,0,0,0,5,-5,-10,0,0,-10,-5,5,5,10,10,-20,-20,10,10,5,0,0,0,0,0,0,0,0]
knight = [-50,-40,-30,-30,-30,-30,-40,-50,-40,-20,0,5,5,0,-20,-40,-30,5,10,15,15,10,5,-30,-30,0,15,20,20,15,0,-30,-30,5,15,20,20,15,5,-30,-30,0,10,15,15,10,0,-30,-40,-20,0,0,0,0,-20,-40,-50,-40,-30,-30,-30,-30,-40,-50]
bishop = [-20,-10,-10,-10,-10,-10,-10,-20,-10,5,0,0,0,0,5,-10,-10,10,10,10,10,10,10,-10,-10,0,10,10,10,10,0,-10,-10,5,5,10,10,5,5,-10,-10,0,5,10,10,5,0,-10,-10,0,0,0,0,0,0,-10,-20,-10,-10,-10,-10,-10,-10,-20]
rook = [0,0,5,10,10,5,0,0,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,5,10,10,10,10,10,10,5,0,0,0,5,5,0,0,0]
queen = [-20,-10,-10,-5,-5,-10,-10,-20,-10,0,5,0,0,0,0,-10,-10,5,5,5,5,5,0,-10,0,0,5,5,5,5,0,-5,-5,0,5,5,5,5,0,-5,-10,0,5,5,5,5,0,-10,-10,0,0,0,0,0,0,-10,-20,-10,-10,-5,-5,-10,-10,-20]
king = [20,30,10,0,0,10,30,20,20,20,0,0,0,0,20,20,-10,-20,-20,-20,-20,-20,-20,-10,-20,-30,-30,-40,-40,-30,-30,-20,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30]

PSTS = {
    0: pawn, 1: knight, 2: bishop, 3: rook, 4: queen, 5: king,
    6: pawn, 7: knight, 8: bishop, 9: rook, 10: queen, 11: king,
}

def pst_value(piece, square):
    if piece < 6:
        return PSTS[piece][square]
    return PSTS[piece][square ^ 56]

def write_network(path):
    input_weights = [0 for _ in range(INPUT_SIZE * HIDDEN_SIZE)]
    hidden_bias = [0 for _ in range(HIDDEN_SIZE)]
    output_weights = [0 for _ in range(HIDDEN_SIZE)]
    output_bias = 0

    # hidden[0] = white score / 16
    # hidden[1] = black score / 16
    output_weights[0] = FEATURE_SCALE * SCALE
    output_weights[1] = -FEATURE_SCALE * SCALE

    for piece in range(12):
        for square in range(64):
            feature = piece * 64 + square
            value = PIECE_VALUES[piece] + pst_value(piece, square)
            scaled = round(value / FEATURE_SCALE)

            if piece < 6:
                input_weights[feature * HIDDEN_SIZE + 0] = scaled
            else:
                input_weights[feature * HIDDEN_SIZE + 1] = scaled

    with open(path, "wb") as f:
        f.write(b"DNNUE001")
        f.write(struct.pack("<i", INPUT_SIZE))
        f.write(struct.pack("<i", HIDDEN_SIZE))

        for x in input_weights:
            f.write(struct.pack("<h", x))

        for x in hidden_bias:
            f.write(struct.pack("<h", x))

        for x in output_weights:
            f.write(struct.pack("<h", x))

        f.write(struct.pack("<i", output_bias))

write_network("tiny_classical.dnnue")
print("wrote tiny_classical.dnnue")