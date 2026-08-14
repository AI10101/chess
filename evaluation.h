#pragma once

#include "board.h"


inline int countMaterial(const Board& board, bool side) {
    int cnt = 0;
    cnt += __builtin_popcountll(board.pieces[side*6 + P]) * 100;
    cnt += __builtin_popcountll(board.pieces[side*6 + N]) * 300;
    cnt += __builtin_popcountll(board.pieces[side*6 + B]) * 330;
    cnt += __builtin_popcountll(board.pieces[side*6 + R]) * 500;
    cnt += __builtin_popcountll(board.pieces[side*6 + Q]) * 900;
    return cnt;
}


int evaluate(const Board& board);