#pragma once

#include "chessboard.h"

using namespace std;

class MoveEngine
{
	private:
		uint64_t nodesSearched;

		int movePly;

		// board evaluation function
		int evaluate(Chessboard& board);

		// minimax search function
		int negamax(Chessboard& board, int depth, int alpha, int beta, int ply);

		int scoreMove(Chessboard& board, Move move, int ply);

		bool sameMove(const Move& a, const Move& b);

		void generateOrderedMoves(Chessboard& board, ScoreMove scoredMoves[], int& numMoves, int ply);

		int quiescence(Chessboard& board, int alpha, int beta);
		
	public:

		static const int MAX_DEPTH = 64;

		Move killerMoves[MAX_DEPTH][2];

		// function to get the best move from a given position
		Move bestMove(Chessboard& board, int depth);

		// returns the number of nodes searched during the last bestMove call
		uint64_t getNodesSearched();

		string moveToString(Move& move);

		void initMoveEngine();
};
