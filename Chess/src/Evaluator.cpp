#include "Evaluator.h"
#include "PieceSquare.h"
#include "bitops.h"
#include <algorithm>
#include <cstdlib>

using namespace std;

const int evaluatorPieceValues[] =
{
	100, 300, 300, 500, 900, 0,
	-100, -300, -300, -500, -900, 0,
	0
};

int Evaluator::classical(Chessboard& board)
{
	PieceSquareTable pst;
	uint64_t* bitboards = board.getState()->bitboards;

	int score = 0;

	for (int i = 0; i < 12; ++i)
	{
		uint64_t piece = bitboards[i];

		while (piece)
		{
			int square = lsbIndex(piece);
			int pstScore = pst.getPieceSquareValue(i, square);

			if (i < 6)
			{
				score += evaluatorPieceValues[i] + pstScore;
			}
			else
			{
				score += evaluatorPieceValues[i] - pstScore;
			}

			piece &= piece - 1;
		}
	}

	return score;
}

int Evaluator::classicalSTM(Chessboard& board)
{
	int score = classical(board);

	return board.turn() == WHITE ? score : -score;
}

int Evaluator::nnueEval(Chessboard& board)
{
	if (!nnue.isLoaded())
	{
		return classical(board);
	}

	return nnue.evaluate(board);
}

int Evaluator::nnueSTM(Chessboard& board)
{
	int score = nnueEval(board);

	return board.turn() == WHITE ? score : -score;
}

int Evaluator::evaluateWhite(Chessboard& board)
{
	int score = useNNUE ? nnueEval(board) : classical(board);

	score += basicMateEndgameBonus(board);

	return score;
}

int Evaluator::evaluateSTM(Chessboard& board)
{
	int score = evaluateWhite(board);

	return board.turn() == WHITE ? score : -score;
}

void Evaluator::setUseNNUE(bool enabled)
{
	useNNUE = enabled;
}

bool Evaluator::isUsingNNUE() const
{
	return useNNUE;
}

bool Evaluator::loadNNUE(const std::string& path)
{
	return nnue.load(path);
}

bool Evaluator::isNNUELoaded() const
{
	return nnue.isLoaded();
}

std::string Evaluator::getNNUEPath() const
{
	return nnue.getNetworkPath();
}

int Evaluator::basicMateEndgameBonus(Chessboard& board)
{
	uint64_t* bb = board.getState()->bitboards;
	int attackerBonus = 0;

	int whiteMaterial =
		popcount(bb[WHITE_QUEEN]) * 900 +
		popcount(bb[WHITE_ROOK]) * 500 +
		popcount(bb[WHITE_BISHOP]) * 300 + 
		popcount(bb[WHITE_KNIGHT]) * 300 +
		popcount(bb[ WHITE_PAWN]) * 100;

	int blackMaterial =
		popcount(bb[BLACK_QUEEN]) * 900 +
		popcount(bb[BLACK_ROOK]) * 500 +
		popcount(bb[BLACK_BISHOP]) * 300 + 
		popcount(bb[BLACK_KNIGHT]) * 300 +
		popcount(bb[BLACK_PAWN]) * 100;

	
	bool whiteHasMateMaterial =
    bb[WHITE_QUEEN] || bb[WHITE_ROOK] ||
    (bb[WHITE_BISHOP] && bb[WHITE_KNIGHT]);

	bool blackHasMateMaterial =
    bb[BLACK_QUEEN] || bb[BLACK_ROOK] ||
    (bb[BLACK_BISHOP] && bb[BLACK_KNIGHT]);

	if (whiteHasMateMaterial && blackMaterial == 0)
	{
		attackerBonus = heavyPieceMateBonus(board, WHITE_QUEEN, WHITE_ROOK, board.blackKingSquare());
		return mateNetBonus(board.whiteKingSquare(), board.blackKingSquare()) + attackerBonus;
	}
	
	if (blackHasMateMaterial && whiteMaterial == 0)
	{
		attackerBonus = heavyPieceMateBonus(board, BLACK_QUEEN, BLACK_ROOK, board.whiteKingSquare());
		return -mateNetBonus(board.blackKingSquare(), board.whiteKingSquare()) + attackerBonus;
	}

	return 0;
}

int Evaluator::heavyPieceMateBonus(Chessboard& board, int queenPiece, int rookPiece, int weakKing)
{
	uint64_t* bb = board.getState()->bitboards;
	
	uint64_t heavyPieces = bb[queenPiece] | bb[rookPiece];

	if(!heavyPieces) return 0;

	int best= 0;

	while(heavyPieces)
	{
		int square = lsbIndex(heavyPieces);

		int file = square % 8;
		int rank = square / 8;
		int weakFile = weakKing % 8;
		int weakRank = weakKing / 8;

		int distance = abs(file - weakFile) + abs(rank - weakRank);

		best = max(best, (14 - distance) * 10);

		heavyPieces &= heavyPieces - 1;
	}

	return best;	
}

int Evaluator::mateNetBonus(int strongKing, int weakKing)
{
	int weakFile = weakKing % 8;
	int weakRank = weakKing / 8;

	int fileEdge = max(weakFile, 7 - weakFile);
	int rankEdge = max(weakRank, 7 - weakRank);
	int edgeBonus = max(fileEdge, rankEdge) * 50;

	int strongFile = strongKing % 8;
	int strongRank = strongKing / 8;

	int kingDistance = abs(strongFile - weakFile) + abs(strongRank - weakRank);
	int kingBonus = (14 - kingDistance) * 20;

	return edgeBonus + kingBonus;
}
