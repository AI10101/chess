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

inline void addMoves(std::vector<move>& moves, bitboard moveMask, int offset, uint16_t flags = 0b0000) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves.push_back((flags << 12) | (sq << 6) | (sq + offset));
    }
}

inline void addMovesFromAttackMask(std::vector<move>& moves, bitboard moveMask, int initialSq, uint16_t flags = 0b0000) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves.push_back((flags << 12) | (sq << 6) | initialSq);
    }
}

inline void addPushPromotions(std::vector<move>& moves, bitboard moveMask, int offset) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves.push_back((0b1000 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1001 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1010 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1011 << 12) | (sq << 6) | (sq + offset));
    }
}

inline void addCapturePromotions(std::vector<move>& moves, bitboard moveMask, int offset) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves.push_back((0b1100 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1101 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1110 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1111 << 12) | (sq << 6) | (sq + offset));
    }
}

std::vector<move> moveGen(Board& board);