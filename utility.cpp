#include "utility.h"

#include "iostream"


void printBoard(Board& board) {
    for (int i = 0; i < 8; i++) std::cout << "+---";
    std::cout << "+\n";

    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            std::cout << "| " << board.board[rank * 8 + file] << " ";
        }
        std::cout << "| " << rank + 1 << "\n";
        for (int i = 0; i < 8; i++) std::cout << "+---";
        std::cout << "+\n";
    }

    for (char c = 'a'; c <= 'h'; c++) {
        std::cout << "  " << c << " ";
    }
    std::cout << "\n";
}

void printBitboard(const bitboard& bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            std::cout << ((bitboard >> sq) & 1ULL);
        }
        std::cout << "\n";
    }
}