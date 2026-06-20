#include "moveEngine.h"
#include "PieceSquare.h"
#include "bitops.h"
#include "cstdlib"
#include "logger.h"
#include <algorithm>
#include <fstream>

using namespace std;

// array of piece values to use for evaluation
const int pieceValues[] = { 100, 300, 300, 500, 900, 0, -100, -300, -300, -500, -900, 0, 0 };

const int capturePieceValues[] = { 100, 300, 300, 500, 900, 10000, 100, 300, 300, 500, 900, 10000 };

bool debugMode = false;

// evaluation function
int MoveEngine::evaluate(Chessboard& board)
{
	PieceSquareTable pst;
	// get the bitboards from the board state
	uint64_t* bitboards = board.getState()->bitboards;

	// loop through all bitboards and accumulate piece scores
	int score = 0;
	for (int i = 0; i < 12; ++i)
	{
		uint64_t piece = bitboards[i];

		while (piece)
		{
			int square = lsbIndex(piece);
			int pstScore = pst.getPieceSquareValue(i, square);

			if (i < 6) 
				score += pieceValues[i] + pstScore;
			else 
				score += pieceValues[i] - pstScore;
				
			piece &= piece - 1;
		}
	}		

	// return the score (positive if white is ahead, negative if black is ahead)
	return score;
}

// Negamax search function
int MoveEngine::negamax(Chessboard& board, int depth, int alpha, int beta, int ply)
{
	++nodesSearched;
	
	// check for terminal states
	int terminal = board.isTerminal();

	if (terminal)
	{
		if (terminal == DRAW) return 0;
		
		return -1000000 + ply; // return a large negative score for a loss, scaled by the depth of the loss
	}

	// depth limit
	if (depth <= 0)
	{
		return board.turn() == WHITE ? evaluate(board) : -evaluate(board);
	} 

	//Logger log("scoreMoves2.txt");
		

	// generate moves
	ScoreMove scoredMoves[218];
	int numMoves;
	
	generateOrderedMoves(board, scoredMoves, numMoves, ply);

	
	// if it is white's turn, try to maximize the score
	int bestScore = -1000000;

	// loop through all moves
	for (int i = 0; i < numMoves; ++i)
	{
		Move move = scoredMoves[i].move;

		bool isCapture = board.pieceAt(move.to) != EMPTY;

		board.move(move);

		bool legal;  
		
		if (board.turn() == BLACK)
		{
			legal = !board.isAttacked(board.whiteKingSquare(), BLACK);
		}
		else
		{
			legal = !board.isAttacked(board.blackKingSquare(), WHITE);
		}

		if (legal)
		{
			// if the move is legal, score it and update best score
			int score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
			//if (score > bestScore) bestScore = score;

			//if(debugMode) log.log("Move: " + moveToString(scoredMoves[i].move) + " Scored: " + to_string(score) + " Depth: " + to_string(depth) + " Nodes: " + to_string(nodesSearched));

			bestScore = max(bestScore, score);
			alpha = max(alpha, bestScore);					
			
			if (beta <= alpha)
			{
				if (!isCapture)
				{
					// if the move is a quiet move, add it to the killer move list
					if (!sameMove(move, killerMoves[ply][0]))
					{
						killerMoves[ply][1] = killerMoves[ply][0];
						killerMoves[ply][0] = move;							
					}
				}
				board.undo();
				break; // beta cut-off
			}
			board.undo();
		}
		else
		{
			board.undo();
		}			
	}
	return bestScore;
	
}

// function to find the best move in a given position
Move MoveEngine::bestMove(Chessboard& board, int depth)
{
	nodesSearched = 0;

	int alpha = -10000000;
	int beta = 10000000;

	// if the state is terminal, return a garbage move
	if (board.isTerminal())
	{
		Move fallback = { 0, 0, EMPTY };
		return fallback;
	}

	// generate moves
	
	ScoreMove scoredMoves[218];
	int numMoves;
	generateOrderedMoves(board, scoredMoves, numMoves, 0);	

	// find the best move
	Move bestMove = scoredMoves[0].move;
	if (board.turn() == WHITE)
	{
		// if it is white's turn, try to maximize the score
		int bestScore = -10000000;

		// loop through moves
		for (int i = 0; i < numMoves; ++i)
		{
			board.move(scoredMoves[i].move);
			if (!board.isAttacked(board.whiteKingSquare(), BLACK))
			{
				// if the move is legal, score it and update bestScore and bestMove
				int score = -negamax(board, depth - 1, -beta, -alpha, 0);
				if (score > bestScore)
				{
					bestScore = score;
					bestMove = scoredMoves[i].move;
				}
			}
			board.undo();
		}
	}
	else
	{
		// if it is black's turn, try to minimize the score
		int bestScore = -10000000;

		// loop through moves
		for (int i = 0; i < numMoves; ++i)
		{
			board.move(scoredMoves[i].move);
			if (!board.isAttacked(board.blackKingSquare(), WHITE))
			{
				// if the move is legal, score it and update bestScore and bestMove
				int score = -negamax(board, depth - 1, -beta, -alpha, 1);
				if (score > bestScore)
				{
					bestScore = score;
					bestMove = scoredMoves[i].move;
				}
			}
			board.undo();
		}
	}

	// return the move that resulted in the best score
	return bestMove;
}

uint64_t MoveEngine::getNodesSearched()
{
	return nodesSearched;
}

int MoveEngine::scoreMove(Chessboard& board, Move move, int ply)
{
	int score = 0;
	//Logger log("scoreMoves2.txt");

	if(sameMove(move, killerMoves[ply][0]))
	{
		score += 9000;
		//if(debugMode) log.log("Move: " + moveToString(move) + " is a killer move");
	}
	else if(sameMove(move, killerMoves[ply][1]))
	{
		score += 8000;
		//if(debugMode) log.log("Move: " + moveToString(move) + " is a second-choice killer move");
	}

	if(move.promotion != EMPTY)
	{			
		score += 20000;// add a bonus for promotion
		//if(debugMode) log.log("Move: " + moveToString(move) + " is a promotion move");
	}	

	int captured = board.pieceAt(move.to);	

	if (captured != EMPTY)
	{
		int victimValue = capturePieceValues[captured];
		int attackerValue = capturePieceValues[board.pieceAt(move.from)];

		score += 10000 + ((10 * victimValue) - attackerValue); // add a bonus for captures, scaled by the value of the captured piece and the attacking piece
		//if(debugMode) log.log("Move: " + moveToString(move) + " is a capture move");

	}

	board.move(move);

	if ((board.isAttacked(board.whiteKingSquare(), BLACK) && board.turn() == WHITE) || (board.isAttacked(board.blackKingSquare(), WHITE) && board.turn() == BLACK))
	{
		score += 5000; // add a bonus for putting the opponent in check
		//if(debugMode) log.log("Move: " + moveToString(move) + " puts the opponent in check");
	}

	board.undo();

	return score;
}

bool MoveEngine::sameMove(const Move& a, const Move& b)
{
	return a.from == b.from && a.to == b.to && a.promotion == b.promotion;
}

void MoveEngine::generateOrderedMoves(Chessboard& board, ScoreMove scoredMoves[], int& numMoves, int ply)
{
	// generate moves
	Move moves[218];
	board.pseudoMoves(moves, numMoves);

	// score all moves
	for (int i = 0; i < numMoves; ++i)
	{
		scoredMoves[i].move = moves[i];
		scoredMoves[i].score = scoreMove(board, moves[i], ply);
	}

	sort(scoredMoves, scoredMoves + numMoves, [](ScoreMove a, ScoreMove b) { return a.score > b.score; });
}

string MoveEngine::moveToString(Move& move)
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

	return moveString;
}

void MoveEngine::initMoveEngine()
{
	Move invalidMove = { 0, 0, EMPTY };

	for (int d = 0; d < MoveEngine::MAX_DEPTH; d++)
	{
		killerMoves[d][0] = invalidMove;
		killerMoves[d][1] = invalidMove;
	}	
}
