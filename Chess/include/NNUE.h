#pragma once

#include "chessBoard.h"
#include <string>

using namespace std;

class NNUE
{
    private:
        bool loaded = false;
        string networkPath = "";

    public:
        bool load(const string& path);

        bool isLoaded() const;

        string getNetworkPath() const;

        int evaluate(Chessboard& board);
};