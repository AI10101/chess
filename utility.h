#pragma once

#include "constants.h"
#include "board.h"


void printBoard(Board& board);

void printBitboard(const bitboard& bitboard);

int popLSB(bitboard& bb);