#pragma once

#include "chessBoard.h"

class Search
{
    private:

        int evaluate(ChessBoard& board); 
        

    public:

        Move bestMove(ChessBoard& board); 
    
};