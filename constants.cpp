#include "constants.h"

#include <cstdint>
#include <immintrin.h>


bitboard rookMagic[64][4096];
bitboard bishopMagic[64][512];

void getRookMagic() {
    for (int sq=0; sq < 64; sq++) {
        bitboard mask;

        bitboard rookMovementMask = rookLookup[sq];

        for (uint64_t pieces = 0; pieces < (1ULL << __builtin_popcountll(rookMovementMask)); pieces++) {
            bitboard board = _pdep_u64(pieces, rookMovementMask);

            bitboard attacks = 0ULL;
            
            // right
            mask = (1ULL << sq) & ~FileH;
            while (mask) {
                mask <<= 1;
                attacks |= mask;
                mask &= ~board;
                mask &= ~FileH;
            }
            // left
            mask = (1ULL << sq) & ~FileA;
            while (mask) {
                mask >>= 1;
                attacks |= mask;
                mask &= ~board;
                mask &= ~FileA;
            }
            // up
            mask = (1ULL << sq) & ~Rank8;
            while (mask) {
                mask <<= 8;
                attacks |= mask;
                mask &= ~board;
                mask &= ~Rank8;
            }
            // down
            mask = (1ULL << sq) & ~Rank1;
            while (mask) {
                mask >>= 8;
                attacks |= mask;
                mask &= ~board;
                mask &= ~Rank1;
            }

            rookMagic[sq][pieces] = attacks;
        }
    }
}

void getBishopMagic() {
    for (int sq=0; sq < 64; sq++) {
        bitboard mask;

        bitboard bishopMovementMask = bishopLookup[sq];

        for (uint64_t pieces = 0; pieces < (1ULL << __builtin_popcountll(bishopMovementMask)); pieces++) {
            bitboard board = _pdep_u64(pieces, bishopMovementMask);

            bitboard attacks = 0ULL;
            
            // up-right
            mask = (1ULL << sq) & ~(FileH | Rank8);
            while (mask) {
                mask <<= 9;
                attacks |= mask;
                mask &= ~board;
                mask &= ~(FileH | Rank8);
            }
            // up-left
            mask = (1ULL << sq) & ~(FileA | Rank8);
            while (mask) {
                mask <<= 7;
                attacks |= mask;
                mask &= ~board;
                mask &= ~(FileA | Rank8);
            }
            // down-right
            mask = (1ULL << sq) & ~(FileH | Rank1);
            while (mask) {
                mask >>= 7;
                attacks |= mask;
                mask &= ~board;
                mask &= ~(FileH | Rank1);
            }
            // down-left
            mask = (1ULL << sq) & ~(FileA | Rank1);
            while (mask) {
                mask >>= 9;
                attacks |= mask;
                mask &= ~board;
                mask &= ~(FileA | Rank1);
            }

            bishopMagic[sq][pieces] = attacks;
        }
    }
}