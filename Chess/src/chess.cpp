#include "chess.h"
#include <iostream>

Board::Board() : whiteToMove(true) {
    // Initialize standard chess board
    char initial[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
    };
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = initial[i][j];
        }
    }
}

void Board::print() const {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            std::cout << board[i][j] << ' ';
        }
        std::cout << std::endl;
    }
}

bool Board::makeMove(const std::string& move) {
    // Simple move parsing, e.g., "e2e4"
    if (move.length() != 4) return false;
    int fromCol = move[0] - 'a';
    int fromRow = 8 - (move[1] - '0');
    int toCol = move[2] - 'a';
    int toRow = 8 - (move[3] - '0');
    if (fromCol < 0 || fromCol > 7 || fromRow < 0 || fromRow > 7 ||
        toCol < 0 || toCol > 7 || toRow < 0 || toRow > 7) return false;
    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = ' ';
    whiteToMove = !whiteToMove;
    return true;
}

std::vector<std::string> Board::generateMoves() const {
    std::vector<std::string> moves;
    // Placeholder: generate pawn moves for white
    for (int col = 0; col < 8; ++col) {
        if (board[6][col] == 'P') {
            // Move forward
            if (board[5][col] == ' ') {
                moves.push_back(std::string(1, 'a'+col) + "7" + std::string(1, 'a'+col) + "6");
            }
        }
    }
    return moves;
}

int Board::evaluate() const {
    // Simple material evaluation
    int score = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            switch (board[i][j]) {
                case 'P': score += 1; break;
                case 'N': score += 3; break;
                case 'B': score += 3; break;
                case 'R': score += 5; break;
                case 'Q': score += 9; break;
                case 'p': score -= 1; break;
                case 'n': score -= 3; break;
                case 'b': score -= 3; break;
                case 'r': score -= 5; break;
                case 'q': score -= 9; break;
            }
        }
    }
    return score;
}