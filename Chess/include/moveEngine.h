#pragma once

#include "chessboard.h"
#include "Evaluator.h"
#include <chrono>
#include <vector>

using namespace std;

struct SearchResult
{
	Move move;
	int score;
	std::uint64_t nodes;
};

enum TTFlag
{
	TT_EXACT,
	TT_LOWERBOUND,
	TT_UPPERBOUND

};

struct TTEntry
{
	uint64_t key = 0;
	int depth = -1;
	int score = 0;
	int flag = TT_EXACT;
	Move bestMove = {0,0,EMPTY};	
};

class MoveEngine
{
	public:
		static const int MAX_DEPTH = 64;

		Move killerMoves[MAX_DEPTH][2];

		// function to get the best move from a given position
		Move bestMove(Chessboard& board, int depth);

		// returns the number of nodes searched during the last bestMove call
		uint64_t getNodesSearched() { return nodesSearched; }

		string moveToString(const Move& move);

		void initMoveEngine();

		Move iterativeDeepening(Chessboard& board, int maxDepth);

		Move iterativeDeepeningTimed(Chessboard& board, int milliseconds);

		SearchResult searchRoot(Chessboard& board, int depth, Move previousBest = Move{0,0,EMPTY});

		uint64_t getTTHits(){return ttHits;}

		uint64_t getTTCuts(){return ttCuts;}

		uint64_t getTTStores(){return ttStores;}

		void clearTT();

		void newSearchGame();

		int staticEval(Chessboard& board);

		int staticEvalWhite(Chessboard& board);

		MoveEngine();

		void setUseNNUE(bool enabled);

		bool isUsingNNUE();

		int nnueEval(Chessboard& board);

		int nnueEvalWhite(Chessboard& board);

		int currentEval(Chessboard& board);

		int currentEvalWhite(Chessboard& board);

		bool loadNNUE(const string& path);

		bool isNNUELoaded();

		string getNNUEPath();

		Move firstLegalMove(Chessboard& board);

	private:
		std::uint64_t nodesSearched;

		int movePly;

		bool stopSearch = false;

		bool useTimeLimit = false;

		chrono::steady_clock::time_point searchDeadline;

		static const int TT_SIZE = 1 <<20;

		vector<TTEntry> transpositionTable;

		uint64_t ttHits = 0;

		uint64_t ttCuts = 0;

		uint64_t ttStores = 0;

		Evaluator evaluator;

		const int DRAW_CONTEMPT = 25;

		static constexpr int MATE_SCORE = 100000;
		static constexpr int MATE_THRESHOLD = MATE_SCORE - MAX_DEPTH;

		static int scoreToTT(int score, int ply);
		static int scoreFromTT(int score, int ply);

		// minimax search function
		int negamax(Chessboard& board, int depth, int alpha, int beta, int ply);

		int scoreMove(Chessboard& board, Move move, int ply, Move ttMove);

		bool sameMove(const Move& a, const Move& b);

		void generateOrderedMoves(Chessboard& board, ScoreMove scoredMoves[], int& numMoves, int ply, Move ttMove);

		int quiescence(Chessboard& board, int alpha, int beta, int qDepth, int ply);

		bool isMoveLegalAfterMove(Chessboard& board);

		bool isNoisyMove(Chessboard& board, const Move& move);
		
		bool shouldStop();
		
		TTEntry* probeTT(uint64_t key);

		void storeTT(uint64_t key, int depth, int score, TTFlag flag, Move bestMove); 

		int drawScore(Chessboard& board);

		int repeatMoveScore(Chessboard& board);	
};
