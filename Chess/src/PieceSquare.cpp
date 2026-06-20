#include "PieceSquare.h"
#include "chessBoard.h"
#include "moveEngine.h"

// Initialize the piece-square tables for each piece type
// These values are typically determined through chess engine tuning and testing
// For simplicity, we will use some example values here

// Example values for pawns
const int pawnMiddleTable[64] =
{
    9000,9000,9000,9000,9000,9000,9000,9000 ,
    200, 200, 200, 200, 200, 200, 200, 200 ,
    100, 100, 100, 100, 100, 100, 100, 100 ,
    40,  40,  90, 100, 100,  90,  40,  40 ,
    20,  20,  20, 100, 150,  20,  20,  20 ,
    2,   4,   0,  15,   4,   0,   4,   2 ,
    -10, -10, -10, -20, -35, -10, -10, -10 ,
    0,   0,   0,   0,   0,   0,   0,   0 
};

const int pawnEndTable[64] =
{
    9000,9000,9000,9000,9000,9000,9000,9000 ,
    500, 500, 500, 500, 500, 500, 500, 500 ,
    300, 300, 300, 300, 300, 300, 300, 300 ,
    90,  90,  90, 100, 100,  90,  90,  90 ,
    70,  70,  70,  85,  85,  70,  70,  70 ,
    20,  20,  20,  20,  20,  20,  20,  20 , 
    -10, -10, -10, -10, -10, -10, -10, -10 ,
    0,   0,   0,   0,   0,   0,   0,   0 
};

const int knightTable[64] =
{
    -20, -80, -60, -60, -60, -60, -80, -20 ,
    -80, -40,   0,   0,   0,   0, -40, -80 ,
    -60,   0,  20,  30,  30,  20,   0, -60 , 
    -60,  10,  30,  40,  40,  30,  10, -60 ,
    -60,   0,  30,  40,  40,  30,   0, -60 , 
    -60,  10,  20,  30,  30,  30,   1, -60 ,
    -80, -40,   0,  10,  10,   0,  -4, -80 ,
    -20, -80, -60, -60, -60, -60, -80, -20 
};

const int bishopTable[64] =
{
    -40, -20, -20, -20, -20, -20, -20, -40 ,
    -20,   0,   0,   0,   0,   0,   0, -20 ,
    -20,   0,  10,  20,  20,  10,   0, -20 ,
    -20,  10,  10,  20,  20,  10,  10, -20 ,
    -20,   0,  20,  20,  20,  20,   0, -20 ,
    -20,  20,  20,  20,  20,  20,  20, -20 ,
    -20,  10,   0,   0,   0,   0,  10, -20 ,
    -40, -20, -20, -20, -20, -20, -20, -40 
};

const int rookTable[64] =
{
    0,  0,  0,  0,  0,  0,  0,   0 ,
    10, 20, 20, 20, 20, 20, 20,  10 ,
    -10,  0,  0,  0,  0,  0,  0, -10 ,
    -10,  0,  0,  0,  0,  0,  0, -10 ,
    -10,  0,  0,  0,  0,  0,  0, -10 ,
    -10,  0,  0,  0,  0,  0,  0, -10 , 
    -10,  0,  0,  0,  0,  0,  0, -10 ,
    -30, 30, 40, 10, 10,  0,  0, -30 
};

const int queenTable[64] =
{
    -40, -20, -20, -10, -10, -20, -20, -40 ,
    -20,   0,   0,   0,   0,   0,   0, -20 ,
    -20,   0,  10,  10,  10,  10,   0, -20 ,
    -10,   0,  10,  10,  10,  10,   0, -10 ,
    0,   0,  10,  10,  10,  10,   0, -10 ,
    -20,  10,  10,  10,  10,  10,   0, -20 ,
    -20,   0,  10,   0,   0,   0,   0, -20 ,
    -40, -20, -20, -10, -10, -20, -20, -40 
};

const int kingMiddleTable[64] =
{
    -60, -80, -80, -2, -20, -80, -80, -60 ,
    -60, -80, -80, -2, -20, -80, -80, -60 ,
    -60, -80, -80, -2, -20, -80, -80, -60 ,
    -60, -80, -80, -2, -20, -80, -80, -60 ,
    -40, -60, -60, -8, -80, -60, -60, -40 ,
    -20, -40, -40, -40,-40, -40, -40, -20 ,
    40,  40,   0,   0,  0,   0,  40,  40 ,
    40,  60,  20,   0,  0,  20,  60,  40 
};

const int kingEndTable[64] =
{
    -175,-175,-175,-175,-175,-175,-175,-175,
    -175, -50, -50, -50, -50, -50, -50,-175,
    -175, -50,  50,  50,  50,  50, -50,-175,
    -175, -50,  50, 150, 150,  50, -50,-175,
    -175, -50,  50, 100, 100,  50, -50,-175,
    -175, -50,  50,  50,  50,  50, -50,-175,
    -175, -50, -50, -50, -50, -50, -50,-175,
    -175,-175,-175,-175,-175,-175,-175,-175
};    


int PieceSquareTable::getPieceSquareValue(int piece, int square)
    {
        switch (piece)
        {
            case WHITE_PAWN:
                return pawnMiddleTable[square];
            case BLACK_PAWN:
                return pawnMiddleTable[square ^ 56];
            case WHITE_KNIGHT:
                return knightTable[square];
            case BLACK_KNIGHT:
                return knightTable[square ^ 56];
            case WHITE_BISHOP:
                return bishopTable[square];
            case BLACK_BISHOP:
                return bishopTable[square ^ 56];
            case WHITE_ROOK:
                return rookTable[square];
            case BLACK_ROOK:
                return rookTable[square ^ 56];
            case WHITE_QUEEN:
                return queenTable[square];
            case BLACK_QUEEN:
                return queenTable[square ^ 56];
            case WHITE_KING:                
                return kingMiddleTable[square];
            case BLACK_KING:
                return kingMiddleTable[square ^ 56];
            default:
                return 0;
        }
    }