#pragma once

#include "board.h"


uint64_t perft(Board& board, int depth);

std::string squareToString(int sq);

std::string moveToString(move m);

uint64_t perftDivide(Board& board, int depth);


void perftTestPosition(std::string name, std::string fen, uint64_t goal, int depth);


void perftTimePosition(std::string name, std::string fen, int depth);


void perftTest();