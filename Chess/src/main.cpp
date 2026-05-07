#include <iostream>
#include "chess.h"

int main() {
    Board board;
    std::cout << "Initial board:" << std::endl;
    board.print();
    std::cout << "Evaluation: " << board.evaluate() << std::endl;
    // Example move
    if (board.makeMove("e2e4")) {
        std::cout << "After e2e4:" << std::endl;
        board.print();
        std::cout << "Evaluation: " << board.evaluate() << std::endl;
    }
    return 0;
}