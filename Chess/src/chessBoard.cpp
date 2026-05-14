#include "chessBoard.h"
#include <iostream>
#include <io.h>
#include <fcntl.h>

using namespace std;

void ChessBoard::print(uint64_t bitBoard) const 
{
    

    for (int rank = 0; rank < 8; rank++) 
    {
        for (int file = 0; file < 8; ++file) 
        {
            int square = rank * 8 + file;

            if (!file)
            {
                wcout << 8 - rank << ' ' << ' ';
            }

            wcout << (getBit(bitBoard, square) ? 1 : 0) << ' ';
        }
        wcout << endl;
    }
    wcout << endl;
    wcout << "   a b c d e f g h" << endl;
    wcout << endl;
    wcout << "   Bitboard: " << bitBoard << endl;
}

ChessBoard::ChessBoard() 
{
    bitBoards[WHITE_PAWN]   = 0x00ff000000000000;
	bitBoards[WHITE_KNIGHT] = 0x4200000000000000;
	bitBoards[WHITE_BISHOP] = 0x2400000000000000;
	bitBoards[WHITE_ROOK]   = 0x8100000000000000;
	bitBoards[WHITE_QUEEN]  = 0x0800000000000000;
	bitBoards[WHITE_KING]   = 0x1000000000000000;
	bitBoards[BLACK_PAWN]   = 0x000000000000ff00;
	bitBoards[BLACK_KNIGHT] = 0x0000000000000024;
	bitBoards[BLACK_BISHOP] = 0x0000000000000042;
	bitBoards[BLACK_ROOK]   = 0x0000000000000081;
	bitBoards[BLACK_QUEEN]  = 0x0000000000000008;
	bitBoards[BLACK_KING]   = 0x0000000000000010;
    
    char_pieces['P'] = P; char_pieces['N'] = N; char_pieces['B'] = B;
    char_pieces['R'] = R; char_pieces['Q'] = Q; char_pieces['K'] = K;
    char_pieces['p'] = p; char_pieces['n'] = n; char_pieces['b'] = b;
    char_pieces['r'] = r; char_pieces['q'] = q; char_pieces['k'] = k;     
}

uint8_t ChessBoard::getPiece(uint8_t square) 
{
    for (int i = 0; i < 12; ++i) 
    {
        uint64_t mask = uint64_t(1) << square;

        if (bitBoards[i] & mask) 
        {
            return i; // Return the piece type index
        }
    }
    return EMPTY; // No piece on this square
}

void ChessBoard::setPiece(uint8_t square, uint8_t piece) 
{
    
        uint64_t mask = uint64_t(1) << square;
        for(int i = 0; i < 12; ++i) 
        {
            if (i == piece) 
            {
                bitBoards[i] |= mask; // Set the bit for the specified piece
            } 
            else 
            {
                bitBoards[i] &= ~mask; // Clear the bit for all other pieces
            }
        }
   
}





