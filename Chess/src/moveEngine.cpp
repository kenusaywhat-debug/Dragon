#include "moveEngine.h"
#include "bitops.h"
#include <iostream>



using namespace std;

//const int pieceValues[12] = {100, 320, 330, 500, 900, 20000, -100, -320, -330, -500, -900, -20000}; // Basic piece values for evaluation
const int pieceValues[] = {100, 300, 300, 500, 900, 0, -100, -300, -300, -500, -900, 0, 0}; // Basic piece values for evaluation

int MoveEngine::evaluate(ChessBoard& board) 
{
    // This function will implement the evaluation function to assess the current board state and return a score.
    uint64_t* bitboards = board.getState()->bitBoards;

    int score = 0;
    for(int i = 0; i < 12; ++i)
    {
        score += popcount(bitboards[i]) * pieceValues[i];
    }
    return score;
}

Move MoveEngine::bestMove(ChessBoard& board, int depth) 
{
    // This function will implement the search algorithm (e.g., minimax with alpha-beta pruning) to find the best move for the current board state.
    // For now, it will just return a placeholder move.

    nodesSearched = 0; // Reset the node count at the start of the search

    if (board.isTerminal()) 
    {
        // If the game is over, return a null move or some indication that there are no moves
        cout << "Terminal position reached. No moves available." << endl;

        Move fallback = {0, 0, EMPTY}; // Placeholder for no move
        return fallback;       
    }

    // Generate moves and evaluate them to find the best move
    Move moves[256];
    int moveCount = 0;
    board.pseudoMoves(moves, moveCount);

    Move bestMove = moves[0]; // Placeholder for the best move
    if(board.turn() == WHITE) 
    {
        int bestScore = -100000; // Initialize best score for white
        for(int i = 0; i < moveCount; ++i) 
        {
            board.move(moves[i]);

            if(!board.isAttacked(board.whiteKingSquare(), BLACK)) 
            {
                int score = minimax(board, depth - 1); // Call minimax to evaluate the move
                if(score > bestScore) 
                {
                    bestScore = score;
                    bestMove = moves[i];
                }
            }
           
            board.undoMove();
        }
    } 
    else 
    {
        int bestScore = 100000; // Initialize best score for black
        for(int i = 0; i < moveCount; ++i) 
        {
            board.move(moves[i]);
            
            if (!board.isAttacked(board.blackKingSquare(), WHITE)) 
            {
                int score = minimax(board, depth - 1); // Call minimax to evaluate the move
                if(score < bestScore) 
                {
                    bestScore = score;
                    bestMove = moves[i];
                }
            }
            board.undoMove();
        }
    }

    cout << "Search complete. Nodes searched: " << nodesSearched << endl;
    return bestMove;
}

int MoveEngine::minimax(ChessBoard& board, int depth) 
{
    // This function will implement the minimax algorithm with alpha-beta pruning to evaluate the game tree and return the best score for the current board state.
    // For now, it will just return a placeholder score.

    ++nodesSearched; // Increment the node count for each call to minimax

    int terminal = board.isTerminal();

    if (depth <= 0) 
    {
        return evaluate(board); // Return the evaluation score for the current board state
    }

    if(terminal)
    {
        if(terminal == WHITE) return 100000 + depth; // White wins
        else if(terminal == BLACK) return -100000 - depth; // Black wins
        else return 0; // Draw
    }
    
    Move moves[256];
    int moveCount;
    board.pseudoMoves(moves, moveCount);

    if(board.turn() == WHITE) 
    {
        int bestScore = -100000; // Initialize best score for white

        for (int i = 0; i < moveCount; ++i)
        {
            board.move(moves[i]);

            if (!board.isAttacked(board.whiteKingSquare(), BLACK)) 
            {
                int score = minimax(board, depth - 1); // Recursive call to minimax

                if (score > bestScore) 
                {
                    bestScore = score; // Update best score for white
                }   
            }
            board.undoMove();
        }        
        return bestScore;
    } 
    else 
    {
       int bestScore = 100000; // Initialize best score for black (minimizing player)

        for (int i = 0; i < moveCount; ++i)
        {
            board.move(moves[i]);

            if (!board.isAttacked(board.blackKingSquare(), WHITE)) 
            {
                int score = minimax(board, depth - 1); // Recursive call to minimax

                if (score < bestScore) 
                {
                    bestScore = score; // Update best score for black
                }   
            }
            board.undoMove();
        }        
        return bestScore;
    }
}

uint64_t MoveEngine::getNodesSearched() 
{
    // This function can be used to track the number of nodes searched during the minimax algorithm
    // For now, it will return 0 as a placeholder
    return nodesSearched;
}