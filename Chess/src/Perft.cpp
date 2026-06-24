#include "Perft.h"
#include "moveEngine.h"
#include <iostream>

uint64_t Perft::perft(Chessboard& board, int depth)
{
    if (depth == 0) return 1;

    Move moves[218];
    int numMoves = 0;

    board.pseudoMoves(moves, numMoves);

    uint64_t nodes = 0;
        
    for (int i = 0; i < numMoves; ++i)
    {
        board.move(moves[i]);

        bool legal = (board.turn() == BLACK ? !board.isAttacked(board.whiteKingSquare(), BLACK) : !board.isAttacked(board.blackKingSquare(), WHITE));
        if (legal)  
        {
            nodes += perft(board, depth - 1);
        }

        board.undo();
    }

    return nodes;
}

uint64_t Perft::perftDivide(Chessboard& board, int depth)
{
    Move moves[218];
    int numMoves = 0;

    board.pseudoMoves(moves, numMoves);
        
    uint64_t totalNodes = 0;

    if (depth == 0) {cout << "Total Nodes: 1" << endl; return 1;}
            
    for (int i = 0; i < numMoves; ++i)
    {
        board.move(moves[i]);

        bool legal = (board.turn() == BLACK ? !board.isAttacked(board.whiteKingSquare(), BLACK) : !board.isAttacked(board.blackKingSquare(), WHITE));
        if (legal)  
        {
            uint64_t nodes = perft(board, depth - 1);
            cout << moveToString(moves[i]) << ": " << nodes << endl;
            totalNodes += nodes;
        }

        board.undo();
    }

    cout << "Total nodes: " << totalNodes << endl;

    return totalNodes;
}

string Perft::moveToString(const Move& move)
{
    // store the move in a string
    int x1 = move.from % 8;
    int y1 = move.from / 8;
    int x2 = move.to % 8;
    int y2 = move.to / 8;

    string moveString = "";
    moveString.push_back(x1 + 'a');
    moveString.push_back(7 + '1' - y1);
    moveString.push_back(x2 + 'a');
    moveString.push_back(7 + '1' - y2);	

    
    switch (move.promotion)
    {
    case WHITE_QUEEN:
    case BLACK_QUEEN:
        moveString.push_back('q');
        break;
    case WHITE_ROOK:
    case BLACK_ROOK:
        moveString.push_back('r');
        break;
    case WHITE_BISHOP:
    case BLACK_BISHOP:
        moveString.push_back('b');
        break;
    case WHITE_KNIGHT:
    case BLACK_KNIGHT:
        moveString.push_back('n');
        break;
    case EMPTY:
    default:
        break;
    }

    return moveString;

}