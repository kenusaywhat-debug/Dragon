#include "chessBoard.h"
#include <iostream>
#include <fstream>
#include <string>


using namespace std;

class Perft
{
    public:
        Perft() = default;

        ~Perft() = default;
        
        uint64_t perft(Chessboard& board, int depth);   

        uint64_t perftDivide(Chessboard& board, int depth);

        string moveToString(const Move& move);
};