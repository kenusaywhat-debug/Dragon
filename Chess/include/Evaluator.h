#pragma once

#include "chessBoard.h"
#include "NNUE.h"
#include <string>

class Evaluator
{
private:
	bool useNNUE = false;
	NNUE nnue;

	int basicMateEndgameBonus(Chessboard& board);
	int heavyPieceMateBonus(Chessboard& board, int queenPiece, int rookPiece, int weakKing);
	int mateNetBonus(int strongKing, int weakKing);


public:
	int classical(Chessboard& board);
	int classicalSTM(Chessboard& board);

	int nnueEval(Chessboard& board);
	int nnueSTM(Chessboard& board);

	int evaluateWhite(Chessboard& board);
	int evaluateSTM(Chessboard& board);

	void setUseNNUE(bool enabled);
	bool isUsingNNUE() const;

	bool loadNNUE(const std::string& path);
	bool isNNUELoaded() const;
	std::string getNNUEPath() const;
};
