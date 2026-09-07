#include "moveEngine.h"
#include "cstdlib"
#include "logger.h"
#include <algorithm>
#include <fstream>

using namespace std;

const int capturePieceValues[] = { 100, 300, 300, 500, 900, 10000, 100, 300, 300, 500, 900, 10000 };

bool debugMode = false;

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
		if (terminal == DRAW)
		{ 
			return 0; // return a small negative score for a draw
		}
		
		return -MATE_SCORE + ply; // return a large negative score for a loss, scaled by the depth of the loss
	}

	if(ply >= MAX_DEPTH)
	{
		stopSearch = true;
		return 0;
	}

	// depth limit
	if (depth <= 0)
	{
		return quiescence(board, alpha, beta, 4, ply);
	} 

	//Logger log("scoreMoves2.txt");

	uint64_t key = board.ttKey();
	int alphaOriginal = alpha;

	Move ttMove = {0, 0, EMPTY};
		
	TTEntry* entry = probeTT(key);

	if (entry != nullptr )
	{
		++ttHits;
		ttMove = entry->bestMove;

		if (entry->depth >= depth)
		{
			int ttScore = scoreFromTT(entry->score, ply);

			if(entry->flag == TT_EXACT)
			{
				++ttCuts;
				return ttScore;
			}
			else if (entry->flag == TT_LOWERBOUND)
			{
				alpha = max(alpha, ttScore);
			}
			else if (entry->flag == TT_UPPERBOUND)
			{
				beta = min(beta, ttScore);
			}

			if (alpha >= beta)
			{
				++ttCuts;
				return ttScore;
			}
		}		
	}

	// generate moves
	ScoreMove scoredMoves[218];
	int numMoves;
	
	generateOrderedMoves(board, scoredMoves, numMoves, ply, ttMove);
	
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
			int	score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);

			if(stopSearch) {board.undo(); return 0; }

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

	storeTT(key, depth, scoreToTT(bestScore, ply), flag, bestMove);
	return bestScore;	
}

int MoveEngine::scoreMove(Chessboard& board, Move move, int ply, Move ttMove)
{
    int score = 0;

    bool hasTTMove =
        !(ttMove.from == 0 &&
          ttMove.to == 0 &&
          ttMove.promotion == EMPTY);

    if (hasTTMove && sameMove(move, ttMove))
    {
        score += 30000;
    }

    if (sameMove(move, killerMoves[ply][0]))
    {
        score += 9000;
    }
    else if (sameMove(move, killerMoves[ply][1]))
    {
        score += 8000;
    }

    if (move.promotion != EMPTY)
    {
        score += 20000;
    }

    int captured = board.pieceAt(move.to);

    if (captured != EMPTY)
    {
        int victimValue = capturePieceValues[captured];
        int attackerValue =
            capturePieceValues[board.pieceAt(move.from)];

        score += 10000 + 10 * victimValue - attackerValue;
    }
    
    return score;
}

// function to find the best move in a given position
Move MoveEngine::bestMove(Chessboard& board, int depth)
{	
	return searchRoot(board, depth).move;
}

SearchResult MoveEngine::searchRoot(Chessboard& board, int depth, Move previousBest)
{
	nodesSearched = 0;

	ttHits = 0;
	ttCuts = 0;
	ttStores = 0;

	int alpha = -10000000;
	int beta = 10000000;
	int bestScore = -10000000;

	Move bestMove = {0, 0, EMPTY};

	ScoreMove scoredMoves[218];
	int numMoves = 0;

	if (board.isTerminal()) return {{0, 0, EMPTY}, 0, nodesSearched};

	Move ttMove = {0, 0, EMPTY};

	TTEntry* entry = probeTT(board.ttKey());

	if (entry != nullptr)
	{
		ttMove = entry->bestMove;
	}

	generateOrderedMoves(board, scoredMoves, numMoves, 0, ttMove);

	for (int i = 0; i < numMoves; ++i)
	{
		if (sameMove(scoredMoves[i].move, previousBest))
		{
			ScoreMove preferred = scoredMoves[i];

			//Preserve the ordering of the remaining moves.
			for(int j = i; j > 0; --j)
			{
				scoredMoves[j] = scoredMoves[j - 1];
			}

			scoredMoves[0] = preferred;
			break;
		}
	}

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

int MoveEngine::drawScore(Chessboard& board)
{
	int eval = evaluator.evaluateSTM(board);

	if (eval >= -25)
	{
		return -DRAW_CONTEMPT; // return a small negative score for a draw
	}

	return 0; // return 0 for a draw
}

int MoveEngine::repeatMoveScore(Chessboard& board)
{
    int evalForSideThatMoved = -evaluator.evaluateSTM(board);

    if (evalForSideThatMoved >= -25)
    {
        return -DRAW_CONTEMPT;
    }

    return 0;
}

int MoveEngine::quiescence(Chessboard& board, int alpha, int beta, int qDepth, int ply)
{
    if (shouldStop()) return 0;

    ++nodesSearched;

    // Keep terminal detection here for correctness first.
    // We can remove duplicate move generation later.
    int terminal = board.isTerminal();

    if (terminal)
    {
        if (terminal == DRAW) return 0;
        return -MATE_SCORE + ply;
    }

    bool inCheck = board.turn() == WHITE ? board.isAttacked(board.whiteKingSquare(), BLACK)
        : board.isAttacked(board.blackKingSquare(), WHITE);

    // Emergency recursion guard: discard this iteration rather
    // than treating a checked position as a normal evaluation.
    if (ply >= MAX_DEPTH)
    {
        stopSearch = true;
        return 0;
    }

    if (!inCheck)
    {
        int standPat = evaluator.evaluateSTM(board);

        if (qDepth <= 0) return standPat;

        if (standPat >= beta) return beta;
        alpha = max(alpha, standPat);
    }

    Move moves[218];
    int numMoves = 0;
    board.pseudoMoves(moves, numMoves);

    for (int i = 0; i < numMoves; ++i)
    {
        Move move = moves[i];

        // In check, quiet evasions must be searched too.
        if (!inCheck && !isNoisyMove(board, move))
            continue;

        board.move(move);

        if (!isMoveLegalAfterMove(board))
        {
            board.undo();
            continue;
        }

        int score = -quiescence(
            board, -beta, -alpha, qDepth - 1, ply + 1);

        board.undo();

        if (stopSearch) return 0;

        if (score >= beta) return beta;
        alpha = max(alpha, score);
    }

    return alpha;
}

bool MoveEngine::sameMove(const Move& a, const Move& b)
{
	return a.from == b.from && a.to == b.to && a.promotion == b.promotion;
}

void MoveEngine::generateOrderedMoves(Chessboard& board, ScoreMove scoredMoves[], int& numMoves, int ply, Move ttMove)
{
	// generate moves
	Move moves[218];
	board.pseudoMoves(moves, numMoves);

	// score all moves
	for (int i = 0; i < numMoves; ++i)
	{
		scoredMoves[i].move = moves[i];
		scoredMoves[i].score = scoreMove(board, moves[i], ply, ttMove);
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
	
	clearTT();
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
	Move best = firstLegalMove(board);
	Move previousBest = {0, 0, EMPTY};

	stopSearch = false;

	if(maxDepth <= 0)
	{		
		return firstLegalMove(board);
	}

	for (int depth = 1; depth <= maxDepth; ++depth)
	{		
		SearchResult result = searchRoot(board, depth, previousBest);

		if (stopSearch){break;}

		best = result.move;
		previousBest = result.move;

		if (result.move.from == 0 && result.move.to == 0 && result.move.promotion == EMPTY)
		{
			cout << "info depth " << depth
				<< " score cp " << result.score
				<< " nodes " << result.nodes
				<< " tthits " << getTTHits()
				<< " ttcuts " << getTTCuts()
				<< " ttstores " << getTTStores()
				<< endl;
		}
		else
		{
			cout << "info depth " << depth
				<< " score cp " << result.score
				<< " nodes " << result.nodes
				<< " tthits " << getTTHits()
				<< " ttcuts " << getTTCuts()
				<< " ttstores " << getTTStores()
				<< " pv " << moveToString(result.move)
				<< endl;
		}
	}

	return best;
}

Move MoveEngine::iterativeDeepeningTimed(Chessboard& board, int milliseconds)
{
	stopSearch = false;
	useTimeLimit = true;
	searchDeadline = chrono::steady_clock::now() + chrono::milliseconds(milliseconds);

	Move best = firstLegalMove(board);
	Move previousBest = {0, 0, EMPTY};

	for(int depth = 1; depth <= MAX_DEPTH; ++depth )
	{
		SearchResult result = searchRoot(board, depth, previousBest);

		if (stopSearch){break;}

		best = result.move;
		previousBest = result.move;

		if (result.move.from == 0 && result.move.to == 0 && result.move.promotion == EMPTY)
		{
			cout << "info depth " << depth
				<< " score cp " << result.score
				<< " nodes " << result.nodes
				<< " tthits " << getTTHits()
				<< " ttcuts " << getTTCuts()
				<< " ttstores " << getTTStores()
				<< endl;
		}
		else
		{
			cout << "info depth " << depth
				<< " score cp " << result.score
				<< " nodes " << result.nodes
				<< " tthits " << getTTHits()
				<< " ttcuts " << getTTCuts()
				<< " ttstores " << getTTStores()
				<< " pv " << moveToString(result.move)
				<< endl;
		}

	}
	useTimeLimit = false;
	return best;
}

bool MoveEngine::shouldStop()
{
	if (stopSearch) { return true;}

	if (useTimeLimit && chrono::steady_clock::now() >= searchDeadline)
	{
		stopSearch = true;
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

int MoveEngine::scoreToTT(int score, int ply)
{
	if (score >= MATE_THRESHOLD) return score + ply;	
	if (score <= -MATE_THRESHOLD) return score - ply;
	return score;	
}

int MoveEngine::scoreFromTT(int score, int ply)
{
	if (score >= MATE_THRESHOLD) return score - ply;	
	if (score <= -MATE_THRESHOLD) return score + ply;
	return score;	
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
		++ttStores;
	}
}

MoveEngine::MoveEngine()
{
	transpositionTable.resize(TT_SIZE);
}

void MoveEngine::clearTT()
{
	fill(transpositionTable.begin(), transpositionTable.end(), TTEntry{});

	ttHits = 0;
	ttCuts = 0;
	ttStores = 0;
}

void MoveEngine::newSearchGame()
{
	clearTT();

	Move invalidMove = {0, 0, EMPTY};

	for(int d = 0; d < MoveEngine::MAX_DEPTH; d++)
	{
		killerMoves[d][0] = invalidMove;
		killerMoves[d][1] = invalidMove;
	}
}

int MoveEngine::staticEval(Chessboard& board)
{
	return evaluator.classicalSTM(board);
}

int MoveEngine::staticEvalWhite(Chessboard& board)
{
	return evaluator.classical(board);
}

int MoveEngine::nnueEval(Chessboard& board)
{
	return evaluator.nnueSTM(board);
}

int MoveEngine::nnueEvalWhite(Chessboard& board)
{
	return evaluator.nnueEval(board);
}

int MoveEngine::currentEval(Chessboard& board)
{
	return evaluator.evaluateSTM(board);
}

int MoveEngine::currentEvalWhite(Chessboard& board)
{
	return evaluator.evaluateWhite(board);
}

void MoveEngine::setUseNNUE(bool enabled)
{
	evaluator.setUseNNUE(enabled);
}

bool MoveEngine::isUsingNNUE()
{
	return evaluator.isUsingNNUE();
}

bool MoveEngine::loadNNUE(const string& path)
{
	bool ok = evaluator.loadNNUE(path);

	if (ok)
	{
		clearTT();
	}

	return ok;
}

bool MoveEngine::isNNUELoaded()
{
	return evaluator.isNNUELoaded();
}

string MoveEngine::getNNUEPath()
{
	return evaluator.getNNUEPath();
}
