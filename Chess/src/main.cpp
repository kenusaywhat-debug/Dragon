#include <iostream>
#include "chessBoard.h"
#include <io.h>
#include <fcntl.h>
#include <Windows.h>

using namespace std;

//ChessBoard game;

int main() 
{
    ChessBoard board;           
        
    board.printBoard(); // Print the entire board with pieces   
    
    return 0;
}