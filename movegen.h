#pragma once

#include "constants.h"
#include "board.h"

#include <vector>


int popLSB(bitboard& bb);

void addMoves(std::vector<move>& moves, bitboard moveMask, int offset, uint16_t flags);

void addMovesFromAttackMask(std::vector<move>& moves, bitboard moveMask, int initialSq, uint16_t flags);

std::vector<move> moveGen(Board& board);

std::vector<move> legalMoveGen(Board& board);