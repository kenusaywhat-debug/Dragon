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

#define WHITE 13
#define BLACK 14

#define DRAW 15

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
 
 inline wchar_t *unicodePieces[12] = {L"\u2659", L"\u2658", L"\u2657", L"\u2656", L"\u2655", L"\u2654",L"\u265F", L"\u265E", L"\u265D", L"\u265C", L"\u265B", L"\u265A"};

 inline int char_pieces[128] = {0};   
 
 struct Move
 {
    uint8_t from;
    uint8_t to;

    uint8_t promotion;

    bool operator== (const Move& other) const 
    {
        return from == other.from && to == other.to && promotion == other.promotion;
    }    
 };

 struct BoardState
 {
    uint64_t bitBoards[12]; // 12 piece types (6 for each color)
    uint8_t turn; // 0 for white, 1 for black
    uint64_t enPassantTarget; // Square index for en passant target
    bool WKC, WQC, BKC, BQC; // castling rights
    uint8_t halfMoves; // For fifty-move rule
    uint64_t zobristHash; // For position hashing
 };
 

class ChessBoard 
{
    public:
        ChessBoard();
        ~ChessBoard();
        void print(uint64_t bitBoard) const;
        void printBoard();

        uint8_t getPiece(uint8_t square);
        void setPiece(uint8_t square, uint8_t piece); 

        BoardState* getState();
        
        uint8_t turn();
        
        void move(const Move& move);
        void undoMove();

        uint8_t isTerminal();
        void pseudoMoves(Move* moves, int& moveCount);

        bool isAttacked(uint8_t square, uint8_t byColor);
        uint8_t whiteKingSquare();
        uint8_t blackKingSquare();

        void setPosition(std::string fen); // Set the board position using a FEN string

        void newGame(); // Reset the board to the initial position


    private:
       
        BoardState* stateStack;
        int stackIndex;

        // Attck bitBoards
        uint64_t knightAttacks[64]; 
        uint64_t kingAttacks[64];
        uint64_t diagonalRays[64][4]; 
        uint64_t cardinalRays[64][4];

        //Zobrist hashing
        uint64_t boardHash[12][64];
        uint64_t passantHash[64];
        uint64_t turnHash;
        uint64_t WKChash;
        uint64_t WQChash;
        uint64_t BKChash;
        uint64_t BQChash;        
        
        bool isLegalMove(const Move& move);
       
        bool softDraw();
        uint64_t zobristHash();
};



#endif