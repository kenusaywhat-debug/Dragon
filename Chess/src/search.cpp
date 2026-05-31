#include "search.h"
#include "chessBoard.h"


using namespace std;

Move Search::bestMove(ChessBoard& board) 
{
    // This function will implement the search algorithm (e.g., minimax with alpha-beta pruning) to find the best move for the current board state.
    // For now, it will just return a placeholder move.
    if (board.isTerminal()) 
    {
        // If the game is over, return a null move or some indication that there are no moves

        Move fallback = {0, 0, EMPTY}; // Placeholder for no move
        return fallback;       
    }

    // Generate moves and evaluate them to find the best move
    Move moves[256];
    int moveCount = 0;
    board.pseudoMoves(moves, moveCount);

    while(moveCount) 
    {
        int r = rand() % moveCount; // Randomly select a move from the generated moves
        board.move(moves[r]);
        bool illegal = board.turn() == WHITE ? board.isAttacked(board.blackKingSquare(), WHITE) : board.isAttacked(board.whiteKingSquare(), BLACK);
        board.undoMove();

        if (!illegal) return moves[r]; // Return the first legal move found

        --moveCount; // Decrement moveCount to account for the last move that was checked and found to be illegal
        moves[r] = moves[moveCount]; // Replace the illegal move with the last move in the list
    } 
    
    
};