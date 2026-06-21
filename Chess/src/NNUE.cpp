#include "NNUE.h"
#include "PieceSquare.h"
#include "bitops.h"
#include <fstream>

const int nnuePieceValues[] =
{
    100, 300, 300, 500, 900, 0,
    -100, -300, -300, -500, -900,
    0, 0
};

bool NNUE::load(const string& path)
{
    ifstream file(path, ios::binary);

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
    PieceSquareTable pst;
    uint64_t* bitboards = board.getState()->bitboards;

    int score = 0;

    for(int i = 0; i < 12; ++i)
    {
        uint64_t piece = bitboards[i];

        while(piece)
        {
            int square = lsbIndex(piece);
            int pstScore = pst.getPieceSquareValue(i, square);

            if(i < 6)
            {
                score += nnuePieceValues[i] + pstScore;
            }
            else
            {
                score += nnuePieceValues[i] - pstScore;
            }

            piece &= piece -1;
        }
    }

    return score;
}