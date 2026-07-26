#pragma once

#include "constants.h"
#include "board.h"


void printBoard(Board& board);

void printBitboard(const bitboard& bitboard);

inline int popLSB(bitboard& bb) {
    int sq = __builtin_ctzll(bb);
    bb &= bb - 1;
    return sq;
}