#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <string>
#include <cstdint>

#define WHITE_PAWN 0
#define WHITE_KNIGHT 1
#define WHITE_BISHOP 2
#define WHITE_ROOK 3
#define WHITE_QUEEN 4
#define WHITE_KING 5

#define BLACK_PAWN 6
#define BLACK_KNIGHT 7
#define BLACK_BISHOP 8
#define BLACK_ROOK 9
#define BLACK_QUEEN 10
#define BLACK_KING 11

#define EMPTY 12

#define getBit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define setBit(bitboard, square) ((bitboard) |= (1ULL << (square))) 
#define clearBit(bitboard, square) (getBit(bitboard, square) ? ((bitboard) &= ~(1ULL << (square))) : 0)

enum
{
 a8, b8, c8, d8, e8, f8, g8, h8,
 a7, b7, c7, d7, e7, f7, g7, h7,
 a6, b6, c6, d6, e6, f6, g6, h6,
 a5, b5, c5, d5, e5, f5, g5, h5,
 a4, b4, c4, d4, e4, f4, g4, h4,
 a3, b3, c3, d3, e3, f3, g3, h3,
 a2, b2, c2, d2, e2, f2, g2, h2,
 a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

inline const char* squareToString[] = 
{
 "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
 "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
 "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
 "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
 "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
 "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
 "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
 "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

 enum{WHITE, BLACK, BOTH};

 enum{WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8};

 enum{P, N, B, R, Q, K, p, n, b, r, q, k};

 inline wchar_t *unicodePieces[12] = {L"\u2659", L"\u2658", L"\u2657", L"\u2656", L"\u2655", L"\u2654",L"\u265F", L"\u265E", L"\u265D", L"\u265C", L"\u265B", L"\u265A"};

 inline int char_pieces[128] = {0};     

class ChessBoard 
{
    public:
        ChessBoard();
        void print(uint64_t bitBoard) const;
        uint8_t getPiece(uint8_t square);
        void setPiece(uint8_t square, uint8_t piece);
        uint64_t bitBoards[12]; // 12 piece types (6 for each color)

    private:
       
        uint64_t occupiedSquares[3]; // Bitboard for all occupied squares
        int sideToMove = -1; // 0 for white, 1 for black
        int enPassantSquare = no_sq; // Square index for en passant target
        int castlingRights = 0; // Bitmask for castling rights
};

#endif