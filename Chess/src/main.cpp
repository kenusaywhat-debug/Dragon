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
    cout << "Initial board:" << std::endl;
    uint64_t bitBoard = 0ULL; // Placeholder for actual bitboard representation

    locale::global(locale("en_US.UTF-8")); // Set global locale to UTF-8
    cout.imbue(locale()); // Imbue cout with the global locale

    SetConsoleOutputCP(CP_UTF8); // Set console output code page to UTF-8
    SetConsoleCP(CP_UTF8); // Set console input code page to UTF-8
    
    //bitBoard |= (1ULL << e2); // Set a piece on e2 for testing

    setBit(board.bitBoards[P], e4); // Example of setting a piece on e2

    board.print(board.bitBoards[P]); // Print the bitboard for white pawns

    wcout << unicodePieces[P] << " is on e4" << endl;

    //clearBit(bitBoard, e4); // Example of clearing a piece from e2

    //board.print(bitBoard);

    //clearBit(bitBoard, e4); // Example of clearing a piece from e2

    //board.print(bitBoard);

    
    //std::cout << "Evaluation: " << board.evaluate() << std::endl;
    // Example move
    // if (board.makeMove("e1e4")) {
    //     std::cout << "After e1e4:" << std::endl;
    //     board.print();
    //     std::cout << "Evaluation: " << board.evaluate() << std::endl;
    // }
    return 0;
}