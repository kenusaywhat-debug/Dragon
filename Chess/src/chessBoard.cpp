#define NOMINMAX
#include "chessBoard.h"
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include "bitops.h"
#include <random>
#include <math.h>

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
    wcout << endl << endl;    
}

void ChessBoard::printBoard()
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

            uint8_t piece = getPiece(square); // Get the piece type index for this square 

            if(piece == EMPTY) 
            {
                wcout << "." << ' ';
            }
            else 
            {
                wcout << unicodePieces[piece] << ' ';
            }
        }
        wcout << endl;
    }
    wcout << endl;
    wcout << "   a b c d e f g h" << endl;
}

uint8_t ChessBoard::getPiece(uint8_t square) 
{
    for (int i = 0; i < 12; ++i) 
    {
        uint64_t mask = uint64_t(1) << square;

        for(int i = 0; i < 12; ++i)
        {
            if (stateStack[stackIndex].bitBoards[i] & mask)
            {
                return i; // Return the piece type index
            }
        }
    }
    return EMPTY; // No piece on this square
}

uint64_t ChessBoard::zobristHash() 
{
    // Implement Zobrist hashing logic here
    uint64_t hash = 0;
    for(int piece = 0; piece < 12; ++piece) 
    {
        uint64_t bitboard = stateStack[stackIndex].bitBoards[piece];
        while(bitboard) 
        {
            uint8_t square = lsbIndex(bitboard);
            hash ^= boardHash[piece][square]; // XOR the hash with the piece-square value
            bitboard &= bitboard - 1; // Clear the least significant bit
        }
    }
    if(stateStack[stackIndex].enPassantTarget) {hash ^= passantHash[lsbIndex(stateStack[stackIndex].enPassantTarget)];} // XOR en passant hash if there is an en passant target
    if(stateStack[stackIndex].turn == WHITE) {hash ^= turnHash;} // XOR turn hash if it's white's turn
    if(!stateStack[stackIndex].WKC) {hash ^= WKChash;} // XOR castling rights hashes if castling rights are lost
    if(!stateStack[stackIndex].WQC) {hash ^= WQChash;} // XOR castling rights hashes if castling rights are lost
    if(!stateStack[stackIndex].BKC) {hash ^= BKChash;} // XOR castling rights hashes if castling rights are lost
    if(!stateStack[stackIndex].BQC) {hash ^= BQChash;} // XOR castling rights hashes if castling rights are lost
    // This should compute and return the hash for the current board state
    return hash;
}

void ChessBoard::setPiece(uint8_t square, uint8_t piece) 
{
    
    uint64_t mask = uint64_t(1) << square;
    for(int i = 0; i < 12; ++i) 
    {
        if (i == piece) 
        {
            stateStack[stackIndex].bitBoards[i] |= mask; // Set the bit for the specified piece
        } 
        else 
        {
            stateStack[stackIndex].bitBoards[i] &= ~mask; // Clear the bit for all other pieces
        }
    }   
}

uint8_t ChessBoard::turn() 
{
    return stateStack[stackIndex].turn;
}

void ChessBoard::move(const Move& move) 
{
    // Implement the logic to make a move on the board
    // This should update the bitboards and other state variables accordingly

    stateStack[stackIndex + 1] = stateStack[stackIndex++]; // Push current state to stack before making the move

    uint64_t fromMask = uint64_t(1) << move.from;
    uint64_t toMask = uint64_t(1) << move.to;
    uint64_t moveBoard = fromMask | toMask;

    uint8_t movePiece = EMPTY;

    for( int i = stateStack[stackIndex].turn == WHITE ? 0 : 6; i < (stateStack[stackIndex].turn == WHITE ? 6 :12); ++i)
    {
        if(stateStack[stackIndex].bitBoards[i] & fromMask) 
        {
            movePiece = i;           
            stateStack[stackIndex].bitBoards[i] ^= moveBoard; // Set the to square
            break;
        }
    }


    //caputre logic
    for(int i = stateStack[stackIndex].turn == WHITE ? 6 : 0; i < (stateStack[stackIndex].turn == WHITE ? 11 : 5); ++i)
    {
        if(stateStack[stackIndex].bitBoards[i] & toMask) 
        {
            stateStack[stackIndex].bitBoards[i] ^= toMask; // Clear the captured piece
            break;
        }
    }

    //en passant capture logic
    if(toMask == stateStack[stackIndex].enPassantTarget) 
    {
        if (movePiece == WHITE_PAWN) 
        {
            stateStack[stackIndex].bitBoards[BLACK_PAWN] ^= stateStack[stackIndex].enPassantTarget << 8;
        }
		else if (movePiece == BLACK_PAWN) 
        {
            stateStack[stackIndex].bitBoards[WHITE_PAWN] ^= stateStack[stackIndex].enPassantTarget >> 8;
        }
    }

    //set en passant target
    if((movePiece == WHITE_PAWN || movePiece == BLACK_PAWN) && abs(move.from - move.to) == 16)
    { 
        stateStack[stackIndex].enPassantTarget = uint64_t(1) << int((move.from + move.to) / 2); 
    }// Set en passant target if it's a double pawn push
    else 
    {
        stateStack[stackIndex].enPassantTarget = 0; 
    }// Clear en passant target if not a double pawn push

    // Castling
    if(movePiece == WHITE_KING) 
    {
        if(moveBoard == 0x5000000000000000) // White king-side castling
        {
            stateStack[stackIndex].bitBoards[WHITE_ROOK] ^= 0xa000000000000000; // Move the rook
        }
        else if(moveBoard == 0x1400000000000000) // White queen-side castling
        {
            stateStack[stackIndex].bitBoards[WHITE_ROOK] ^= 0x9000000000000000; // Move the rook
        }
    }
    else if(movePiece == BLACK_KING) 
    {
        if(moveBoard == 0x0000000000000050) // Black king-side castling
        {
            stateStack[stackIndex].bitBoards[BLACK_ROOK] ^= 0x00000000000000a0; // Move the rook
        }
        else if(moveBoard == 0x0000000000000014) // Black queen-side castling
        {
            stateStack[stackIndex].bitBoards[BLACK_ROOK] ^= 0x0000000000000009; // Move the rook
        }
    }

    // update castling rights
    if(movePiece == WHITE_KING) 
    {
        stateStack[stackIndex].WKC = false;
        stateStack[stackIndex].WQC = false;
    }
    else if(movePiece == BLACK_KING) 
    {
        stateStack[stackIndex].BKC = false;
        stateStack[stackIndex].BQC = false;
    }
    else if(movePiece == WHITE_ROOK) 
    {
        if(move.from == h1) {stateStack[stackIndex].WKC = false;}
        else if(move.from == a1) {stateStack[stackIndex].WQC = false;}
    }
    else if(movePiece == BLACK_ROOK) 
    {
        if(move.from == h8) {stateStack[stackIndex].BKC= false;}
        else if(move.from == a8) {stateStack[stackIndex].BQC = false;}
    }

    // Handle promotion
    if(move.promotion != EMPTY) 
    {
        stateStack[stackIndex].bitBoards[movePiece] ^= toMask; // Clear the piece from the to square
        stateStack[stackIndex].bitBoards[move.promotion] |= toMask; // Set the promoted piece on the to square
    }    
}

void ChessBoard::undoMove() 
{
    // Implement the logic to undo the last move
    // This should restore the previous state from the stack

    if(stackIndex > 0) 
    {
        --stackIndex; // Move back to the previous state
    }
}   

void ChessBoard::pseudoMoves(Move* moves, int& moveCount) 
{
    // Generate pseudo-legal moves for the current position
    // This function should populate the moves array and update moveCount
    moveCount = 0;

    uint64_t whiteBoard = 0;
    uint64_t blackBoard = 0;
    for(int i = 0; i < 6; ++i) {whiteBoard |= stateStack[stackIndex].bitBoards[i];}
    for(int i = 6; i < 12; ++i) {blackBoard |= stateStack[stackIndex].bitBoards[i];}
    uint64_t board = whiteBoard | blackBoard;

    //White Moves
    if(stateStack[stackIndex].turn == WHITE) 
    {
        // Generate moves for white pieces
        // This is where you would implement the move generation logic for each piece type
        // For example, you would generate moves for pawns, knights, bishops, rooks, queens, and kings
        // You would also need to handle special moves like castling and en passant

        //pawn moves
        uint64_t bb = (stateStack[stackIndex].bitBoards[WHITE_PAWN] >> 8) & ~board; // Single push
        uint64_t doublePush = ((stateStack[stackIndex].bitBoards[WHITE_PAWN] >> 16) & ~board) & (0x0000ff0000000000); // Double push from starting position
        uint8_t square;
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            uint8_t origin = square + 8; // Original square of the pawn
            if(square < 8) // Handle promotion
            {
                moves[moveCount++] = {origin, square, WHITE_QUEEN}; // Promote to queen
                moves[moveCount++] = {origin, square, WHITE_ROOK};  // Promote to rook
                moves[moveCount++] = {origin, square, WHITE_BISHOP}; // Promote to bishop
                moves[moveCount++] = {origin, square, WHITE_KNIGHT}; // Promote to knight
            }
            else
            {
                moves[moveCount++] = {origin, square, EMPTY}; // Add move to moves array 
            }
                       
        }
        while (doublePush)
        {
            square = lsbIndex(doublePush);
            doublePush &= doublePush - 1; // Clear the least significant bit
            uint8_t origin = square + 16; // Original square of the pawn
            moves[moveCount++] = {origin, square, EMPTY}; // Add move to moves array            
        }

        //pawn attack moves
        bb = ((stateStack[stackIndex].bitBoards[WHITE_PAWN] & 0xfefefefefefefefe) >> 9) & (blackBoard | stateStack[stackIndex].enPassantTarget); // Capture to the right
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            uint8_t origin = square + 9; // Original square of the pawn
            if(square < 8) // Handle promotion
            {
                moves[moveCount++] = {origin, square, WHITE_QUEEN}; // Promote to queen
                moves[moveCount++] = {origin, square, WHITE_ROOK};  // Promote to rook
                moves[moveCount++] = {origin, square, WHITE_BISHOP}; // Promote to bishop
                moves[moveCount++] = {origin, square, WHITE_KNIGHT}; // Promote to knight
            }
            else
            {
                moves[moveCount++] = {origin, square, EMPTY}; // Add move to moves array            
            }
        }
        bb = ((stateStack[stackIndex].bitBoards[WHITE_PAWN] & 0x7f7f7f7f7f7f7f7f) >> 7) & (blackBoard | stateStack[stackIndex].enPassantTarget); // Capture to the left
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            uint8_t origin = square + 7; // Original square of the pawn
            if(square < 8) // Handle promotion
            {
                moves[moveCount++] = {origin, square, WHITE_QUEEN}; // Promote to queen
                moves[moveCount++] = {origin, square, WHITE_ROOK};  // Promote to rook
                moves[moveCount++] = {origin, square, WHITE_BISHOP}; // Promote to bishop
                moves[moveCount++] = {origin, square, WHITE_KNIGHT}; // Promote to knight
            }
            else
            {
                moves[moveCount++] = {origin, square, EMPTY}; // Add move to moves array            
            }
        }

        //diagonal moves for bishops and queens would be generated here using the diagonalRays attack tables
        bb = stateStack[stackIndex].bitBoards[WHITE_BISHOP] | stateStack[stackIndex].bitBoards[WHITE_QUEEN];
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
           
            uint64_t diagonalMoves = 0;
            for(int dir = 0; dir < 4; ++dir) 
            {
                diagonalMoves |= diagonalRays[square][dir]; // Get diagonal moves from precomputed tables
                uint64_t blockers = diagonalRays[square][dir] & board; // Get blockers in the path
                if(blockers) 
                {
                    int blockerIndex;
                    if(dir == 0 || dir == 3) blockerIndex = lsbIndex(blockers);
                    else blockerIndex = msbIndex(blockers);
                    diagonalMoves &= ~(diagonalRays[blockerIndex][dir]); // Remove moves beyond the blocker
                }                
            }
            diagonalMoves &= ~whiteBoard; // Remove friendly pieces from moves

            uint8_t target;
            while(diagonalMoves) 
            {
                target = lsbIndex(diagonalMoves);
                diagonalMoves &= diagonalMoves - 1; // Clear the least significant bit
                moves[moveCount++] = {square, target, EMPTY}; // Add move to moves array            
            }
        }


        //cardinal moves for rooks and queens would be generated here using the cardinalRays attack tables
        bb = stateStack[stackIndex].bitBoards[WHITE_ROOK] | stateStack[stackIndex].bitBoards[WHITE_QUEEN];
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
           
            uint64_t cardinalMoves = 0;
            for(int dir = 0; dir < 4; ++dir) 
            {
                cardinalMoves |= cardinalRays[square][dir]; // Get cardinal moves from precomputed tables
                uint64_t blockers = cardinalRays[square][dir] & board; // Get blockers in the path
                if(blockers) 
                {
                    int blockerIndex;
                    if(dir == 0 || dir == 2) blockerIndex = lsbIndex(blockers);
                    else blockerIndex = msbIndex(blockers);
                    cardinalMoves &= ~(cardinalRays[blockerIndex][dir]); // Remove moves beyond the blocker
                }                
            }
            cardinalMoves &= ~whiteBoard; // Remove friendly pieces from moves

            uint8_t target;
            while(cardinalMoves) 
            {
                target = lsbIndex(cardinalMoves);
                cardinalMoves &= cardinalMoves - 1; // Clear the least significant bit
                moves[moveCount++] = {square, target, EMPTY}; // Add move to moves array            
            }
        }


        // Knigth moves
        bb = stateStack[stackIndex].bitBoards[WHITE_KNIGHT];    
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            uint64_t attacks = knightAttacks[square] & ~whiteBoard; // Get knight attacks and remove friendly pieces
            uint8_t origin = square; // Original square of the knight

            while(attacks) 
            {
                uint8_t targetSquare = lsbIndex(attacks);
                attacks &= attacks - 1; // Clear the least significant bit
                moves[moveCount++] = {origin, targetSquare, EMPTY}; // Add move to moves array            
            }
        }

        // King moves
        square = lsbIndex(stateStack[stackIndex].bitBoards[WHITE_KING]);
        bb = kingAttacks[square] & ~whiteBoard; // Get king attacks and remove friendly pieces
        while (bb)  
        {
            uint8_t targetSquare = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            moves[moveCount++] = {square, targetSquare, EMPTY}; // Add move to moves array            
        }

        // Castling moves
        if (stateStack[stackIndex].WKC && !(board & 0x6000000000000000) && !isAttacked(60, BLACK) && !isAttacked(61, BLACK)) moves[moveCount++] = { 60, 62, EMPTY };
		if (stateStack[stackIndex].WQC && !(board & 0x0e00000000000000) && !isAttacked(60, BLACK) && !isAttacked(59, BLACK)) moves[moveCount++] = { 60, 58, EMPTY };
    }
    else 
    {
        // Generate moves for black pieces
        // Similar to white, but for black pieces

        //pawn moves
        uint64_t bb = (stateStack[stackIndex].bitBoards[BLACK_PAWN] << 8) & ~board; // Single push
        uint64_t doublePush = ((stateStack[stackIndex].bitBoards[BLACK_PAWN] << 16) & ~board) & (0x0000000000ff0000); // Double push from starting position
        uint8_t square;
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            uint8_t origin = square - 8; // Original square of the pawn
            if(square >= 56) // Handle promotion
            {
                moves[moveCount++] = {origin, square, BLACK_QUEEN}; // Promote to queen
                moves[moveCount++] = {origin, square, BLACK_ROOK};  // Promote to rook
                moves[moveCount++] = {origin, square, BLACK_BISHOP}; // Promote to bishop
                moves[moveCount++] = {origin, square, BLACK_KNIGHT}; // Promote to knight
            }
            else
            {   
                moves[moveCount++] = {origin, square, EMPTY}; // Add move to moves array            
            }
        }
        while (doublePush)
        {
            square = lsbIndex(doublePush);
            doublePush &= doublePush - 1; // Clear the least significant bit
            uint8_t origin = square - 16; // Original square of the pawn
            moves[moveCount++] = {origin, square, EMPTY}; // Add move to moves array            
        }

        //pawn attack moves
        bb = ((stateStack[stackIndex].bitBoards[BLACK_PAWN] & 0xfefefefefefefefe) << 7) & (whiteBoard | stateStack[stackIndex].enPassantTarget); // Capture to the right
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            uint8_t origin = square - 7; // Original square of the pawn
            if(square >= 56) // Handle promotion
            {
                moves[moveCount++] = {origin, square, BLACK_QUEEN}; // Promote to queen
                moves[moveCount++] = {origin, square, BLACK_ROOK};  // Promote to rook
                moves[moveCount++] = {origin, square, BLACK_BISHOP}; // Promote to bishop
                moves[moveCount++] = {origin, square, BLACK_KNIGHT}; // Promote to knight
            }
            else
            {
                moves[moveCount++] = {origin, square, EMPTY}; // Add move to moves array            
            }
        }

        bb = ((stateStack[stackIndex].bitBoards[BLACK_PAWN] & 0x7f7f7f7f7f7f7f7f) << 9) & (whiteBoard | stateStack[stackIndex].enPassantTarget); // Capture to the left
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            uint8_t origin = square - 9; // Original square of the pawn
            if(square >= 56) // Handle promotion
            {
                moves[moveCount++] = {origin, square, BLACK_QUEEN}; // Promote to queen
                moves[moveCount++] = {origin, square, BLACK_ROOK};  // Promote to rook
                moves[moveCount++] = {origin, square, BLACK_BISHOP}; // Promote to bishop
                moves[moveCount++] = {origin, square, BLACK_KNIGHT}; // Promote to knight
            }
            else
            {
                moves[moveCount++] = {origin, square, EMPTY}; // Add move to moves array            
            }                      
        }

        //diagonal moves for bishops and queens would be generated here using the diagonalRays attack tables
        bb = stateStack[stackIndex].bitBoards[BLACK_BISHOP] | stateStack[stackIndex].bitBoards[BLACK_QUEEN];
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
           
            uint64_t diagonalMoves = 0;
            for(int dir = 0; dir < 4; ++dir) 
            {
                diagonalMoves |= diagonalRays[square][dir]; // Get diagonal moves from precomputed tables
                uint64_t blockers = diagonalRays[square][dir] & board; // Get blockers in the path
                if(blockers) 
                {
                    int blockerIndex;
                    if(dir == 0 || dir == 3) blockerIndex = lsbIndex(blockers);
                    else blockerIndex = msbIndex(blockers);
                    diagonalMoves &= ~(diagonalRays[blockerIndex][dir]); // Remove moves beyond the blocker
                }                
            }
            diagonalMoves &= ~blackBoard; // Remove friendly pieces from moves

            uint8_t target;
            while(diagonalMoves) 
            {
                target = lsbIndex(diagonalMoves);
                diagonalMoves &= diagonalMoves - 1; // Clear the least significant bit
                moves[moveCount++] = {square, target, EMPTY}; // Add move to moves array            
            }
        }

        //cardinal moves for rooks and queens would be generated here using the cardinalRays attack tables
        bb = stateStack[stackIndex].bitBoards[BLACK_ROOK] | stateStack[stackIndex].bitBoards[BLACK_QUEEN];
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
           
            uint64_t cardinalMoves = 0;
            for(int dir = 0; dir < 4; ++dir) 
            {
                cardinalMoves |= cardinalRays[square][dir]; // Get cardinal moves from precomputed tables
                uint64_t blockers = cardinalRays[square][dir] & board; // Get blockers in the path
                if(blockers) 
                {
                    int blockerIndex;
                    if(dir == 0 || dir == 2) blockerIndex = lsbIndex(blockers);
                    else blockerIndex = msbIndex(blockers);
                    cardinalMoves &= ~(cardinalRays[blockerIndex][dir]); // Remove moves beyond the blocker
                }                
            }
            cardinalMoves &= ~blackBoard; // Remove friendly pieces from moves

            uint8_t target;
            while(cardinalMoves) 
            {
                target = lsbIndex(cardinalMoves);
                cardinalMoves &= cardinalMoves - 1; // Clear the least significant bit
                moves[moveCount++] = {square, target, EMPTY}; // Add move to moves array            
            }
        }

        // Knigth moves
        bb = stateStack[stackIndex].bitBoards[BLACK_KNIGHT];    
        while(bb) 
        {
            square = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            uint64_t attacks = knightAttacks[square] & ~blackBoard; // Get knight attacks and remove friendly pieces
            uint8_t origin = square; // Original square of the knight

            while(attacks) 
            {
                uint8_t targetSquare = lsbIndex(attacks);
                attacks &= attacks - 1; // Clear the least significant bit
                moves[moveCount++] = {origin, targetSquare, EMPTY}; // Add move to moves array            
            }
        }

        // King moves
        square = lsbIndex(stateStack[stackIndex].bitBoards[BLACK_KING]);
        bb = kingAttacks[square] & ~blackBoard; // Get king attacks and remove friendly pieces
        while (bb)  
        {
            uint8_t targetSquare = lsbIndex(bb);
            bb &= bb - 1; // Clear the least significant bit
            moves[moveCount++] = {square, targetSquare, EMPTY}; // Add move to moves array            
        }

        // Castling moves
        if (stateStack[stackIndex].BKC && !(board & 0x0000000000000060) && !isAttacked(4, WHITE) && !isAttacked(5, WHITE)) moves[moveCount++] = { 4, 6, EMPTY };
		if (stateStack[stackIndex].BQC && !(board & 0x000000000000000e) && !isAttacked(4, WHITE) && !isAttacked(3, WHITE)) moves[moveCount++] = { 4, 2, EMPTY };	
    }
}

bool ChessBoard::isLegalMove(const Move& move) 
{
    // Check if the given move is legal in the current position
    // This function should return true if the move is legal, false otherwise
    Move moves[256]; // Array to hold pseudo-legal moves
    int moveCount = 0;    
    pseudoMoves(moves, moveCount); // Generate pseudo-legal moves

    for(int i = 0; i < moveCount; ++i) 
    {
        if(move == moves[i]) 
        {
            this->move(moves[i]);
			bool illegal = turn() == WHITE ? isAttacked(blackKingSquare(), WHITE) : isAttacked(whiteKingSquare(), BLACK);
			undoMove();
			if (!illegal) return true;
        }
    }
    return false;
}

bool ChessBoard::isAttacked(uint8_t square, uint8_t byColor) 
{
    if(byColor == BLACK) 
    {
        uint64_t board = 0;
        for (int i = 0; i < 12; ++i) {board |= stateStack[stackIndex].bitBoards[i];} // Get white pieces on the board


        //Cardinally attacked
        uint64_t attackers = stateStack[stackIndex].bitBoards[BLACK_QUEEN] | stateStack[stackIndex].bitBoards[BLACK_ROOK];
        for(int dir = 0; dir < 4; ++dir) 
        {
            uint64_t blockers = cardinalRays[square][dir] & board;;
            if(blockers & attackers) 
            {
                uint64_t blockerIndex;
                if(dir == 0 || dir == 2) blockerIndex = lsbIndex(blockers);
                else blockerIndex = msbIndex(blockers);
                if(attackers & (uint64_t(1) << blockerIndex)) 
                {
                    return true; // Square is attacked by a rook or queen
                }
            }
        }

        // diagonally attacked
		attackers = stateStack[stackIndex].bitBoards[BLACK_QUEEN] | stateStack[stackIndex].bitBoards[BLACK_BISHOP];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = diagonalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 3) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

        // attacked by a knight
		if (knightAttacks[square] & stateStack[stackIndex].bitBoards[BLACK_KNIGHT]) return true;

		// attacked by a pawn
		if ((((stateStack[stackIndex].bitBoards[BLACK_PAWN] & 0xfefefefefefefefe) << 7) | ((stateStack[stackIndex].bitBoards[BLACK_PAWN] & 0x7f7f7f7f7f7f7f7f) << 9)) & (uint64_t(1) << square)) return true;
	
		// attacked by a king
		if (kingAttacks[square] & stateStack[stackIndex].bitBoards[BLACK_KING]) return true;
    }
    else if (byColor == WHITE)
	{
		uint64_t board = 0;
		for (int i = 0; i < 12; ++i) board |= stateStack[stackIndex].bitBoards[i];

		// cardinally attacked
		uint64_t attackers = stateStack[stackIndex].bitBoards[WHITE_QUEEN] | stateStack[stackIndex].bitBoards[WHITE_ROOK];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = cardinalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 2) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

		// diagonally attacked
		attackers = stateStack[stackIndex].bitBoards[WHITE_QUEEN] | stateStack[stackIndex].bitBoards[WHITE_BISHOP];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = diagonalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 3) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

        // attacked by a knight
		if (knightAttacks[square] & stateStack[stackIndex].bitBoards[WHITE_KNIGHT]) return true;

		// attacked by a pawn
		if ((((stateStack[stackIndex].bitBoards[WHITE_PAWN] & 0xfefefefefefefefe) >> 9) | ((stateStack[stackIndex].bitBoards[WHITE_PAWN] & 0x7f7f7f7f7f7f7f7f) >> 7)) & (uint64_t(1) << square)) return true;

		// attacked by a king
		if (kingAttacks[square] & stateStack[stackIndex].bitBoards[WHITE_KING]) return true;
	}

	return false;
}

uint8_t ChessBoard::whiteKingSquare()
{
    return lsbIndex(stateStack[stackIndex].bitBoards[WHITE_KING]);
}

uint8_t ChessBoard::blackKingSquare()
{
    return lsbIndex(stateStack[stackIndex].bitBoards[BLACK_KING]);
}

bool ChessBoard::softDraw()
{
    if (stateStack[stackIndex].halfMoves >= 100) return true; // Fifty-move rule
    
    int n =0;

    for (int i = stackIndex -1; i >=0; --i)
    {
        if (stateStack[stackIndex].zobristHash == stateStack[i].zobristHash) 
        {
            n++;
            if (n >= 2) return true; // Threefold repetition
        }
    }

    if (stateStack[stackIndex].bitBoards[WHITE_PAWN] || stateStack[stackIndex].bitBoards[BLACK_PAWN] || 
        stateStack[stackIndex].bitBoards[WHITE_ROOK] || stateStack[stackIndex].bitBoards[BLACK_ROOK] || 
        stateStack[stackIndex].bitBoards[WHITE_QUEEN] || stateStack[stackIndex].bitBoards[BLACK_QUEEN]) 
    {
        return false; // Not a draw by insufficient material
    }

    uint64_t white_bb = stateStack[stackIndex].bitBoards[WHITE_BISHOP] | stateStack[stackIndex].bitBoards[WHITE_KNIGHT];
    white_bb &= white_bb - 1; // Clear the least significant bit to check if there's more than one minor piece
    uint64_t black_bb = stateStack[stackIndex].bitBoards[BLACK_BISHOP] | stateStack[stackIndex].bitBoards[BLACK_KNIGHT];
    black_bb &= black_bb - 1; // Clear the least significant bit to check if there's more than one minor piece
    if (!white_bb && !black_bb) {return true; } // King vs King
    return false;
}

uint8_t ChessBoard::isTerminal()
{
    if (softDraw()) return DRAW; // Draw
    Move moves[256];
    int moveCount = 0;
    pseudoMoves(moves, moveCount);

    for(int i = 0; i < moveCount; ++i) 
    {
        this->move(moves[i]);
        bool illegal = turn() == WHITE ? isAttacked(blackKingSquare(), WHITE) : isAttacked(whiteKingSquare(), BLACK);
        undoMove();
        if (!illegal) return 0; // Found a legal move, so not terminal
    }

    if (turn() == WHITE) 
    {
        return isAttacked(whiteKingSquare(), BLACK) ? BLACK : DRAW; // Checkmate or stalemate
    }
    else 
    {
        return isAttacked(blackKingSquare(), WHITE) ? WHITE : DRAW; // Checkmate or stalemate
    }
}

ChessBoard::ChessBoard() 
{
    // Set up Random Zobrist keys for hashing
    random_device rd;
    mt19937_64 rng(rd());
    uniform_int_distribution<uint64_t> dist(0, numeric_limits<uint64_t>::max());

    for(int square = 0; square < 64; ++square) 
    {
        for(int piece = 0; piece < 12; ++piece) 
        {
            boardHash[piece][square] = dist(rng);
        }
    }
    turnHash = dist(rng);
    WKChash = dist(rng);
    WQChash = dist(rng);
    BKChash = dist(rng);
    BQChash = dist(rng);
    
    stateStack = new BoardState[1000]; // Allocate memory for 100 board states (for move history)
    stackIndex = 0; // Start at the initial position

    stateStack[0].bitBoards[WHITE_PAWN]   = 0x00ff000000000000;
    stateStack[0].bitBoards[WHITE_KNIGHT] = 0x4200000000000000;
    stateStack[0].bitBoards[WHITE_BISHOP] = 0x2400000000000000;
    stateStack[0].bitBoards[WHITE_ROOK]   = 0x8100000000000000;
    stateStack[0].bitBoards[WHITE_QUEEN]  = 0x0800000000000000;
    stateStack[0].bitBoards[WHITE_KING]   = 0x1000000000000000;
    stateStack[0].bitBoards[BLACK_PAWN]   = 0x000000000000ff00;
    stateStack[0].bitBoards[BLACK_KNIGHT] = 0x0000000000000024;
    stateStack[0].bitBoards[BLACK_BISHOP] = 0x0000000000000042;
    stateStack[0].bitBoards[BLACK_ROOK]   = 0x0000000000000081;
    stateStack[0].bitBoards[BLACK_QUEEN]  = 0x0000000000000008;
    stateStack[0].bitBoards[BLACK_KING]   = 0x0000000000000010;

    stateStack[0].WKC = true; 
    stateStack[0].WQC = true; 
    stateStack[0].BKC = true; 
    stateStack[0].BQC = true;

    stateStack[0].enPassantTarget = 0; // No en passant target square in the initial position

    stateStack[0].turn = WHITE; // White to move

    // Generate attack bitboards
    for(int x = 0; x < 8; ++x) 
    {
        for(int y = 0; y < 8; ++y) 
        {
            uint8_t squareIndex = x + 8 * y;

			// knight attacks
			knightAttacks[squareIndex] = 0;
			if (x < 7 && y < 6) knightAttacks[squareIndex] |= uint64_t(1) << (x + 1 + 8 * (y + 2));
			if (x < 6 && y < 7) knightAttacks[squareIndex] |= uint64_t(1) << (x + 2 + 8 * (y + 1));
			if (x < 7 && y > 1) knightAttacks[squareIndex] |= uint64_t(1) << (x + 1 + 8 * (y - 2));
			if (x > 1 && y < 7) knightAttacks[squareIndex] |= uint64_t(1) << (x - 2 + 8 * (y + 1));
			if (x < 6 && y > 0) knightAttacks[squareIndex] |= uint64_t(1) << (x + 2 + 8 * (y - 1));
			if (x > 0 && y < 6) knightAttacks[squareIndex] |= uint64_t(1) << (x - 1 + 8 * (y + 2));
			if (x > 0 && y > 1) knightAttacks[squareIndex] |= uint64_t(1) << (x - 1 + 8 * (y - 2));
			if (x > 1 && y > 0) knightAttacks[squareIndex] |= uint64_t(1) << (x - 2 + 8 * (y - 1));


            // king attacks
			kingAttacks[squareIndex] = 0;
			if (x < 7 && y < 7) kingAttacks[squareIndex] |= uint64_t(1) << (x + 1 + 8 * (y + 1));
			if (x < 7) kingAttacks[squareIndex] |= uint64_t(1) << (x + 1 + 8 * y);
			if (x < 7 && y > 0) kingAttacks[squareIndex] |= uint64_t(1) << (x + 1 + 8 * (y - 1));
			if (y < 7) kingAttacks[squareIndex] |= uint64_t(1) << (x + 8 * (y + 1));
			if (y > 0) kingAttacks[squareIndex] |= uint64_t(1) << (x + 8 * (y - 1));
			if (x > 0 && y < 7) kingAttacks[squareIndex] |= uint64_t(1) << (x - 1 + 8 * (y + 1));
			if (x > 0) kingAttacks[squareIndex] |= uint64_t(1) << (x - 1 + 8 * y);
			if (x > 0 && y > 0) kingAttacks[squareIndex] |= uint64_t(1) << (x - 1 + 8 * (y - 1));

            // diagonal attack rays
			diagonalRays[squareIndex][0] = 0;
			for (int x2 = x + 1, y2 = y + 1; x2 < 8 && y2 < 8; ++x2, ++y2) diagonalRays[squareIndex][0] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalRays[squareIndex][1] = 0;
			for (int x2 = x + 1, y2 = y - 1; x2 < 8 && y2 >= 0; ++x2, --y2) diagonalRays[squareIndex][1] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalRays[squareIndex][2] = 0;
			for (int x2 = x - 1, y2 = y - 1; x2 >= 0 && y2 >= 0; --x2, --y2) diagonalRays[squareIndex][2] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalRays[squareIndex][3] = 0;
			for (int x2 = x - 1, y2 = y + 1; x2 >= 0 && y2 < 8; --x2, ++y2) diagonalRays[squareIndex][3] |= uint64_t(1) << (x2 + 8 * y2);

            // cardinal attack rays
			cardinalRays[squareIndex][0] = 0;
			for (int x2 = x + 1; x2 < 8; ++x2) cardinalRays[squareIndex][0] |= uint64_t(1) << (x2 + 8 * y);
			cardinalRays[squareIndex][1] = 0;
			for (int x2 = x - 1; x2 >= 0; --x2) cardinalRays[squareIndex][1] |= uint64_t(1) << (x2 + 8 * y);
			cardinalRays[squareIndex][2] = 0;
			for (int y2 = y + 1; y2 < 8; ++y2) cardinalRays[squareIndex][2] |= uint64_t(1) << (x + 8 * y2);
			cardinalRays[squareIndex][3] = 0;
			for (int y2 = y - 1; y2 >= 0; --y2) cardinalRays[squareIndex][3] |= uint64_t(1) << (x + 8 * y2);
        }
    }



    
    locale::global(locale("en_US.UTF-8")); // Set global locale to UTF-8
    cout.imbue(locale()); // Imbue cout with the global locale

    SetConsoleOutputCP(CP_UTF8); // Set console output code page to UTF-8
    SetConsoleCP(CP_UTF8); // Set console input code page to UTF-8
}

ChessBoard::~ChessBoard() 
{
    // Clean up any dynamically allocated memory if necessary
    delete[] stateStack;
}
