#include "constants.h"

#include <cstdint>
#include <immintrin.h>


bitboard const Rank1 = 0xff;
bitboard const Rank2 = 0xff00;
bitboard const Rank7 = 0xff000000000000;
bitboard const Rank8 = 0xff00000000000000;

bitboard const FileA = 0x101010101010101;
bitboard const FileB = 0x202020202020202;
bitboard const FileG = 0x4040404040404040;
bitboard const FileH = 0x8080808080808080;

bitboard kingLookup[64];
bitboard knightLookup[64];
bitboard rookLookup[64];
bitboard bishopLookup[64];

void getKingLookup() {
    for (int sq=0; sq < 64; sq++) {
        bitboard mask = 1ULL << sq;
        bitboard attacks = 0ULL;

        attacks |= (mask & ~FileH) << 1; // right
        attacks |= (mask & ~FileA) >> 1; // left
        attacks |= (mask & ~Rank8) << 8; // up
        attacks |= (mask & ~Rank1) >> 8; // down
        attacks |= ((mask & ~FileH) & ~Rank8) << 9; // right-up
        attacks |= ((mask & ~FileA) & ~Rank8) << 7; // left-up
        attacks |= ((mask & ~FileH) & ~Rank1) >> 7; // right-down
        attacks |= ((mask & ~FileA) & ~Rank1) >> 9; // left-down

        kingLookup[sq] = attacks;
    }
}

void getKnightLookup() {
    for (int sq=0; sq < 64; sq++) {
        bitboard mask = 1ULL << sq;
        bitboard attacks = 0ULL;
        
        attacks |= (mask & ~(Rank8 | FileA | FileB)) << 6;  // left-up
        attacks |= (mask & ~(FileA | Rank8 | Rank7)) << 15; // up-left
        attacks |= (mask & ~(FileH | Rank8 | Rank7)) << 17; // up-right
        attacks |= (mask & ~(Rank8 | FileH | FileG)) << 10; // right-up
        attacks |= (mask & ~(Rank1 | FileH | FileG)) >> 6;  // right-down
        attacks |= (mask & ~(FileH | Rank1 | Rank2)) >> 15; // down-right
        attacks |= (mask & ~(FileA | Rank1 | Rank2)) >> 17; // down-left
        attacks |= (mask & ~(Rank1 | FileA | FileB)) >> 10; // left-down

        knightLookup[sq] = attacks;
    }
}

void getRookLookup() {
    for (int sq=0; sq < 64; sq++) {
        bitboard mask;
        bitboard attacks = 0ULL;
        
        // right
        mask = (1ULL << sq) & ~FileH;
        while (mask) {
            mask <<= 1;
            mask &= ~FileH;
            attacks |= mask;
        }
        // left
        mask = (1ULL << sq) & ~FileA;
        while (mask) {
            mask >>= 1;
            mask &= ~FileA;
            attacks |= mask;
        }
        // up
        mask = (1ULL << sq) & ~Rank8;
        while (mask) {
            mask <<= 8;
            mask &= ~Rank8;
            attacks |= mask;
        }
        // down
        mask = (1ULL << sq) & ~Rank1;
        while (mask) {
            mask >>= 8;
            mask &= ~Rank1;
            attacks |= mask;
        }

        rookLookup[sq] = attacks;
    }
}

void getBishopLookup() {
    for (int sq=0; sq < 64; sq++) {
        bitboard mask;
        bitboard attacks = 0ULL;
        
        // up-right
        mask = (1ULL << sq) & ~(FileH | Rank8);
        while (mask) {
            mask <<= 9;
            mask &= ~(FileH | Rank8);
            attacks |= mask;
        }
        // up-left
        mask = (1ULL << sq) & ~(FileA | Rank8);
        while (mask) {
            mask <<= 7;
            mask &= ~(FileA | Rank8);
            attacks |= mask;
        }
        // down-right
        mask = (1ULL << sq) & ~(FileH | Rank1);
        while (mask) {
            mask >>= 7;
            mask &= ~(FileH | Rank1);
            attacks |= mask;
        }
        // down-left
        mask = (1ULL << sq) & ~(FileA | Rank1);
        while (mask) {
            mask >>= 9;
            mask &= ~(FileA | Rank1);
            attacks |= mask;
        }

        bishopLookup[sq] = attacks;
    }
}

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
};

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
};