#pragma once

#include "chessBoard.h"

class MoveEngine
{
    private:

        uint64_t nodesSearched; // For tracking the number of nodes searched during the minimax algorithm

        int evaluate(ChessBoard& board); 

        int minimax(ChessBoard& board, int depth); //, int alpha, int beta, bool maximizingPlayer);
        

    public:

        Move bestMove(ChessBoard& board, int depth); 

        uint64_t getNodesSearched();
    
};