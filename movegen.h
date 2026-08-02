#pragma once

#include "constants.h"
#include "board.h"
#include "board.h"

#include <vector>


inline int popLSB(bitboard& bb) {
    int sq = __builtin_ctzll(bb);
    bb &= bb - 1;
    return sq;
}

inline void addMoves(move* moves, int& cnt, bitboard moveMask, int offset, uint16_t flags = 0b0000) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((flags << 12) | (sq << 6) | (sq + offset));
    }
}

inline void addMovesFromAttackMask(move* moves, int& cnt, bitboard moveMask, int initialSq, uint16_t flags = 0b0000) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((flags << 12) | (sq << 6) | initialSq);
    }
}

inline void addPushPromotions(move* moves, int& cnt, bitboard moveMask, int offset) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((0b1000 << 12) | (sq << 6) | (sq + offset));
        moves[cnt++] = ((0b1001 << 12) | (sq << 6) | (sq + offset));
        moves[cnt++] = ((0b1010 << 12) | (sq << 6) | (sq + offset));
        moves[cnt++] = ((0b1011 << 12) | (sq << 6) | (sq + offset));
    }
}

inline void addCapturePromotions(move* moves, int& cnt, bitboard moveMask, int offset) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((0b1100 << 12) | (sq << 6) | (sq + offset));
        moves[cnt++] = ((0b1101 << 12) | (sq << 6) | (sq + offset));
        moves[cnt++] = ((0b1110 << 12) | (sq << 6) | (sq + offset));
        moves[cnt++] = ((0b1111 << 12) | (sq << 6) | (sq + offset));
    }
}

int moveGen(Board& board, move* moves);