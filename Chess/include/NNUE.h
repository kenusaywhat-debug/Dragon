#pragma once

#include "chessBoard.h"
#include <string>
#include <vector>
#include <cstdint>

using namespace std;

class NNUE
{
    private:
        static const int INPUT_SIZE = 12 * 64;
        static const int HIDDEN_SIZE = 128;
        static const int SCALE = 256;
    
        bool loaded = false;
        string networkPath = "";

        vector<int16_t> inputWeights;
        vector<int16_t> hiddenBias;
        vector<int16_t> outputWeights;
        int32_t outputBias = 0;

        int featureIndex(int poece, int square) const;

    public:
        NNUE();

        bool load(const string& path);
        bool isLoaded() const;
        string getNetworkPath() const;

        int evaluate(Chessboard& board);
};