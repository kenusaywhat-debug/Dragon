#ifndef CHESS_H
#define CHESS_H

#include <string>
#include <vector>

class Board {
public:
    Board();
    void print() const;
    bool makeMove(const std::string& move);
    std::vector<std::string> generateMoves() const;
    // Simple evaluation
    int evaluate() const;

private:
    char board[8][8]; // ' ' for empty, 'P' pawn, etc.
    bool whiteToMove;
};

#endif