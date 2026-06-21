#include "Evaluator.h"
#include "PieceSquare.h"
#include "bitops.h"

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
	return useNNUE ? nnueEval(board) : classical(board);
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
