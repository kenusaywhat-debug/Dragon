#include "NNUE.h"
#include "PieceSquare.h"
#include "bitops.h"
#include <fstream>
#include <algorithm>

// const int nnuePieceValues[] =
// {
//     100, 300, 300, 500, 900, 0,
//     -100, -300, -300, -500, -900,
//     0, 0
// };

NNUE::NNUE()
{
    inputWeights.resize(INPUT_SIZE * HIDDEN_SIZE, 0);
    hiddenBias.resize(HIDDEN_SIZE, 0);
    outputWeights.resize(HIDDEN_SIZE, 0);
}

int NNUE::featureIndex(int piece, int square) const
{
    return piece * 64 + square;
}

bool NNUE::load(const string& path)
{
    ifstream file(path, ios::binary);

    if(!file)
    {
        loaded = false;
        networkPath = "";
        return false;
    }

    char magic[8];
    file.read(magic, 8);

    if(string(magic, 8) != "DNNUE001")
    {
        loaded = false;
        networkPath = "";
        return false;
    }
    
    int32_t inputSize = 0;
    int32_t hiddenSize = 0;

    file.read(reinterpret_cast<char*>(&inputSize), sizeof(inputSize));
    file.read(reinterpret_cast<char*>(&hiddenSize), sizeof(hiddenSize));

    if(inputSize != INPUT_SIZE || hiddenSize != HIDDEN_SIZE)
    {
        loaded = false;
        networkPath = "";
        return false;
    }

    file.read(reinterpret_cast<char*>(inputWeights.data()), inputWeights.size() * sizeof(int16_t));
    file.read(reinterpret_cast<char*>(hiddenBias.data()), hiddenBias.size() * sizeof(int16_t));
	file.read(reinterpret_cast<char*>(outputWeights.data()), outputWeights.size() * sizeof(int16_t));
	file.read(reinterpret_cast<char*>(&outputBias), sizeof(outputBias));

    if(!file)
    {
        loaded = false;
        networkPath = "";
        return false;
    }

    loaded = true;
    networkPath = path;
    return true;

}

bool NNUE::isLoaded() const
{
    return loaded;
}

string NNUE::getNetworkPath() const
{
    return networkPath;
}

int NNUE::evaluate(Chessboard& board)
{
    vector<int32_t> hidden(HIDDEN_SIZE, 0);

    for(int i = 0; i < HIDDEN_SIZE; ++i)
    {
        hidden[i] = hiddenBias[i];
    }

    uint64_t* bitboards = board.getState()->bitboards;

    for(int piece = 0; piece < 12; ++piece)
    {
        uint64_t pieces = bitboards[pieces];

        while (pieces)
        {
            int square = lsbIndex(pieces);
            int feature = featureIndex(piece, square);

            for (int h = 0; h < HIDDEN_SIZE; ++h)
            {
                hidden[h] += inputWeights[feature * HIDDEN_SIZE + h];
            }

            pieces &= pieces -1;
        }
    }

    int32_t score = outputBias;

    for(int h = 0; h < HIDDEN_SIZE; ++h)
    {
        int32_t activated = clamp(hidden[h], 0, 255);
        score += activated * outputWeights[h];
    }

    return score / SCALE;
}