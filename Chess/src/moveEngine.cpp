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
	if(shouldStop()){return 0;}

	++nodesSearched;

	Move bestMove = {0, 0, EMPTY};
	
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
		return quiescence(board, alpha, beta);
	} 

	//Logger log("scoreMoves2.txt");

	uint64_t key = board.getState()->hash;
	int alphaOriginal = alpha;
		
	TTEntry* entry = probeTT(key);

	if (entry != nullptr && entry->depth >= depth)
	{
		if(entry->flag == TT_EXACT)
		{
			return entry->score;
		}
		else if (entry->flag == TT_LOWERBOUND)
		{
			alpha = max(alpha, entry->score);
		}
		else if (entry->flag == TT_UPPERBOUND)
		{
			beta = min(beta, entry->score);
		}

		if (alpha >= beta)
		{
			return entry->score;
		}
	}
	


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
		
		if (isMoveLegalAfterMove(board))
		{
			// if the move is legal, score it and update best score
			int score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
			//if (score > bestScore) bestScore = score;

			//if(debugMode) log.log("Move: " + moveToString(scoredMoves[i].move) + " Scored: " + to_string(score) + " Depth: " + to_string(depth) + " Nodes: " + to_string(nodesSearched));
			if (score > bestScore)
			{
				bestScore = score;
				bestMove = move;
			}
			
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
		if (stopSearch)
		{
			return alpha;
		}			
	}

	TTFlag flag = TT_EXACT;

	if(bestScore <= alphaOriginal)
	{
		flag = TT_UPPERBOUND;
	}
	else if(bestScore >= beta)
	{
		flag = TT_LOWERBOUND;
	}

	storeTT(key, depth, bestScore, flag, bestMove);
	return bestScore;
	
}

// function to find the best move in a given position
Move MoveEngine::bestMove(Chessboard& board, int depth)
{	
	return searchRoot(board, depth).move;
}

int MoveEngine::quiescence(Chessboard& board, int alpha, int beta)
{
	if (shouldStop()){ return board.turn() == WHITE ? evaluate(board) : -evaluate(board);}

	++nodesSearched;
	
	int standPat = board.turn() == WHITE ? evaluate(board) : -evaluate(board);

	if (standPat >= beta) return beta;
	if (alpha < standPat) alpha = standPat;

	// generate capture moves
	Move moves[218];
	int numMoves = 0;
	board.pseudoMoves(moves, numMoves);

	for (int i = 0; i < numMoves; ++i)
	{
		Move move = moves[i];

		if(!isNoisyMove(board, move)){continue;} // skip quiet moves
		
		board.move(move);		

		if (isMoveLegalAfterMove(board))
		{
			int score = -quiescence(board, -beta, -alpha);
			board.undo();

			if(stopSearch){ return alpha;}

			if (score >= beta) return beta;
			if (score > alpha) alpha = score;
		}
		else
		{
			board.undo();
			if(stopSearch){ return alpha;}

		}		
	}

	return alpha;
}

uint64_t MoveEngine::getNodesSearched()
{
	return nodesSearched;
}

int MoveEngine::scoreMove(Chessboard& board, Move move, int ply)
{
	int score = 0;
	//Logger log("scoreMoves2.txt");

	TTEntry* entry = probeTT(board.getState()->hash);
	Move ttMove = {0, 0, EMPTY};

	if (entry != nullptr)
	{
		ttMove = entry->bestMove;
	}

	if(sameMove(move, ttMove))
	{
		score += 30000;
	}

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

string MoveEngine::moveToString(const Move& move)
{// store the move in a string
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

void MoveEngine::initMoveEngine()
{
	Move invalidMove = { 0, 0, EMPTY };

	for (int d = 0; d < MoveEngine::MAX_DEPTH; d++)
	{
		killerMoves[d][0] = invalidMove;
		killerMoves[d][1] = invalidMove;
	}	
}

bool MoveEngine::isMoveLegalAfterMove(Chessboard& board)
{
	if (board.turn() == BLACK)
	{
		return !board.isAttacked(board.whiteKingSquare(), BLACK);
	}
	else
	{
		return !board.isAttacked(board.blackKingSquare(), WHITE);
	}
}

bool MoveEngine::isNoisyMove(Chessboard& board, const Move& move)
{
	// a move is considered noisy if it is a capture or a promotion
	if (board.pieceAt(move.to) != EMPTY || move.promotion != EMPTY)
	{
		return true;
	}

	uint8_t piece = board.pieceAt(move.from);

	bool isPawn = (piece == WHITE_PAWN || piece == BLACK_PAWN);

	bool isEnPassant = (isPawn && board.getState()->passantTarget && 
		((uint64_t(1) << move.to) == board.getState()->passantTarget));

	return isEnPassant;
}

Move MoveEngine::iterativeDeepening(Chessboard& board, int maxDepth)
{
	Move best = {0, 0, EMPTY};

	stopSearch = false;

	if(maxDepth <= 0)
	{		
		return firstLegalMove(board);
	}

	for (int depth = 1; depth <= maxDepth; ++depth)
	{		
		SearchResult result = searchRoot(board, depth);
		best = result.move;

		cout << "info depth " << depth << " score cp " << result.score << " nodes " << result.nodes << " pv " << moveToString(result.move) << endl;
	}

	return best;
}

SearchResult MoveEngine::searchRoot(Chessboard& board, int depth)
{
	nodesSearched = 0;

	int alpha = -10000000;
	int beta = 10000000;
	int bestScore = -10000000;

	Move bestMove = {0, 0, EMPTY};

	ScoreMove scoredMoves[218];
	int numMoves = 0;

	if (board.isTerminal()) return {{0, 0, EMPTY}, 0, nodesSearched};

	generateOrderedMoves(board, scoredMoves, numMoves, 0);

	bool foundLegal = false;

	for (int i = 0; i < numMoves; ++i)
	{
		Move move = scoredMoves[i].move;

		board.move(move); 

		if (isMoveLegalAfterMove(board))
		{
			foundLegal = true;

			int score = -negamax(board, depth - 1, -beta, -alpha, 1);

			if(stopSearch) {board.undo(); break; }

			if (score > bestScore)
			{
				bestScore = score;
				bestMove = move;
				alpha = max(alpha, bestScore);
			}
		}		
		board.undo();		
	}
	
	if (!foundLegal)
	{
		return {{0, 0, EMPTY}, bestScore, nodesSearched};
	}

	return {bestMove, bestScore, nodesSearched};
}

Move MoveEngine::iterativeDeepeningTimed(Chessboard& board, int milliseconds)
{
	stopSearch = false;
	useTimeLimit = true;
	searchDeadline = chrono::steady_clock::now() + chrono::milliseconds(milliseconds);

	Move best = firstLegalMove(board);

	for(int depth = 1; depth <= MAX_DEPTH; ++depth )
	{
		SearchResult result = searchRoot(board, depth);

		if (stopSearch){break;}

		best = result.move;

		cout << "info depth " << depth << " score cp " << result.score << " nodes "<< result.nodes << " pv " << moveToString(result.move) << endl;

	}
	useTimeLimit = false;
	return best;
}

bool MoveEngine::shouldStop()
{
	if (!useTimeLimit) { return false;}

	if (chrono::steady_clock::now() >= searchDeadline)
	{
		stopSearch = true;
		return true;
	}

	return stopSearch;
}

Move MoveEngine::firstLegalMove(Chessboard& board)
{
	Move moves[218];
	int numMoves = 0;
	board.pseudoMoves(moves, numMoves);

	for(int i = 0; i < numMoves; ++i)
	{
		Move move = moves[i];

		board.move(move);

		if(isMoveLegalAfterMove(board))
		{
			board.undo();
			return move;
		}
		board.undo();
	}

	return{0, 0, EMPTY};
}

TTEntry* MoveEngine::probeTT(uint64_t key)
{
	int index = key & (TT_SIZE -1);
	TTEntry& entry = transpositionTable[index];

	if (entry.key == key)
	{
		return &entry;
	}

	return nullptr;
}

void MoveEngine::storeTT(uint64_t key, int depth, int score, TTFlag flag, Move bestMove)
{
	int index = key & (TT_SIZE -1);
	TTEntry& entry = transpositionTable[index];

	if (entry.key == 0 || depth >= entry.depth)
	{
		entry.key = key;
		entry.depth = depth;
		entry.score = score;
		entry.flag = flag;
		entry.bestMove = bestMove;
	}
}
