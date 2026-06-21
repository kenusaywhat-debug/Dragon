#pragma once

#include "chessboard.h"
#include <chrono>

using namespace std;

struct SearchResult
{
	Move move;
	int score;
	std::uint64_t nodes;
};


class MoveEngine
{
	private:
		std::uint64_t nodesSearched;

		int movePly;

		bool stopSearch;

		bool useTimeLimit;

		chrono::steady_clock::time_point searchDeadline;

		// board evaluation function
		int evaluate(Chessboard& board);

		// minimax search function
		int negamax(Chessboard& board, int depth, int alpha, int beta, int ply);

		int scoreMove(Chessboard& board, Move move, int ply);

		bool sameMove(const Move& a, const Move& b);

		void generateOrderedMoves(Chessboard& board, ScoreMove scoredMoves[], int& numMoves, int ply);

		int quiescence(Chessboard& board, int alpha, int beta);

		bool isMoveLegalAfterMove(Chessboard& board);

		bool isNoisyMove(Chessboard& board, const Move& move);
		
		bool shouldStop();
		
	public:

		static const int MAX_DEPTH = 64;

		Move killerMoves[MAX_DEPTH][2];

		// function to get the best move from a given position
		Move bestMove(Chessboard& board, int depth);

		// returns the number of nodes searched during the last bestMove call
		uint64_t getNodesSearched();

		string moveToString(const Move& move);

		void initMoveEngine();

		Move iterativeDeepening(Chessboard& board, int maxDepth);

		Move iterativeDeepeningTimed(Chessboard& board, int milliseconds);

		SearchResult searchRoot(Chessboard& board, int depth);
};
