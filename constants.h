#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <random>


using bitboard = uint64_t;
using move = uint16_t;


const bitboard Rank1 = 0xff;
const bitboard Rank2 = 0xff00;
const bitboard Rank7 = 0xff000000000000;
const bitboard Rank8 = 0xff00000000000000;

const bitboard FileA = 0x101010101010101;
const bitboard FileB = 0x202020202020202;
const bitboard FileG = 0x4040404040404040;
const bitboard FileH = 0x8080808080808080;


constexpr std::array<uint8_t, 64> initCastlingUpdateLookup() {
    std::array<uint8_t, 64> lookup{};

    // in most cases moving or capturing the piece does not affect castling rights
    for (int sq=0; sq<64; sq++) lookup[sq] = 0b1111;

    // cK, cQ, ck, cq;
    lookup[4]  = 0b0011; // moving white king
    lookup[60] = 0b1100; // moving black king
    lookup[0]  = 0b1011; // moving or capturing white queen-side rook 
    lookup[7]  = 0b0111; // moving or capturing white king-side rook
    lookup[56] = 0b1110; // moving or capturing black queen-side rook
    lookup[63] = 0b1101; // moving or capturing black king-side rook

    return lookup;
}

constexpr std::array<uint8_t, 64> castlingUpdate = initCastlingUpdateLookup();


constexpr std::array<bitboard, 64> initKingLookup() {
    std::array<bitboard, 64> lookup{};

    for (int sq=0; sq < 64; sq++) {
        bitboard piece = 1ULL << sq;
        bitboard attacks = 0ULL;

        attacks |= (piece & ~FileH) << 1; // right
        attacks |= (piece & ~FileA) >> 1; // left
        attacks |= (piece & ~Rank8) << 8; // up
        attacks |= (piece & ~Rank1) >> 8; // down
        attacks |= (piece & ~FileH & ~Rank8) << 9; // right-up
        attacks |= (piece & ~FileA & ~Rank8) << 7; // left-up
        attacks |= (piece & ~FileH & ~Rank1) >> 7; // right-down
        attacks |= (piece & ~FileA & ~Rank1) >> 9; // left-down

        lookup[sq] = attacks;
    }

    return lookup;
}

constexpr std::array<bitboard, 64> initKnightLookup() {
    std::array<bitboard, 64> lookup{};

    for (int sq=0; sq < 64; sq++) {
        bitboard piece = 1ULL << sq;
        bitboard attacks = 0ULL;
        
        attacks |= (piece & ~(Rank8 | FileA | FileB)) << 6;  // left-up
        attacks |= (piece & ~(FileA | Rank8 | Rank7)) << 15; // up-left
        attacks |= (piece & ~(FileH | Rank8 | Rank7)) << 17; // up-right
        attacks |= (piece & ~(Rank8 | FileH | FileG)) << 10; // right-up
        attacks |= (piece & ~(Rank1 | FileH | FileG)) >> 6;  // right-down
        attacks |= (piece & ~(FileH | Rank1 | Rank2)) >> 15; // down-right
        attacks |= (piece & ~(FileA | Rank1 | Rank2)) >> 17; // down-left
        attacks |= (piece & ~(Rank1 | FileA | FileB)) >> 10; // left-down

        lookup[sq] = attacks;
    }

    return lookup;
}

constexpr std::array<bitboard, 64> initRookLookup() {
    std::array<bitboard, 64> lookup{};

    for (int sq=0; sq < 64; sq++) {
        bitboard piece = 0ULL;
        bitboard attacks = 0ULL;
        
        // right
        piece = (1ULL << sq) & ~FileH;
        while (piece) {
            piece <<= 1;
            piece &= ~FileH;
            attacks |= piece;
        }
        // left
        piece = (1ULL << sq) & ~FileA;
        while (piece) {
            piece >>= 1;
            piece &= ~FileA;
            attacks |= piece;
        }
        // up
        piece = (1ULL << sq) & ~Rank8;
        while (piece) {
            piece <<= 8;
            piece &= ~Rank8;
            attacks |= piece;
        }
        // down
        piece = (1ULL << sq) & ~Rank1;
        while (piece) {
            piece >>= 8;
            piece &= ~Rank1;
            attacks |= piece;
        }

        lookup[sq] = attacks;
    }

    return lookup;
}

constexpr std::array<bitboard, 64> initBishopLookup() {
    std::array<bitboard, 64> lookup{};

    for (int sq=0; sq < 64; sq++) {
        bitboard piece = 0ULL;
        bitboard attacks = 0ULL;
        
        // up-right
        piece = (1ULL << sq) & ~(FileH | Rank8);
        while (piece) {
            piece <<= 9;
            piece &= ~(FileH | Rank8);
            attacks |= piece;
        }
        // up-left
        piece = (1ULL << sq) & ~(FileA | Rank8);
        while (piece) {
            piece <<= 7;
            piece &= ~(FileA | Rank8);
            attacks |= piece;
        }
        // down-right
        piece = (1ULL << sq) & ~(FileH | Rank1);
        while (piece) {
            piece >>= 7;
            piece &= ~(FileH | Rank1);
            attacks |= piece;
        }
        // down-left
        piece = (1ULL << sq) & ~(FileA | Rank1);
        while (piece) {
            piece >>= 9;
            piece &= ~(FileA | Rank1);
            attacks |= piece;
        }

        lookup[sq] = attacks;
    }

    return lookup;
}

constexpr std::array<bitboard, 64> kingLookup = initKingLookup();
constexpr std::array<bitboard, 64> knightLookup = initKnightLookup();
constexpr std::array<bitboard, 64> rookLookup = initRookLookup();
constexpr std::array<bitboard, 64> bishopLookup = initBishopLookup();


extern bitboard rookMagic[64][4096];
extern bitboard bishopMagic[64][512];

void getRookMagic();
void getBishopMagic();


extern std::mt19937_64 rng;

extern uint64_t pieceSqKey[12][64];
extern uint64_t sideToMoveKey;
extern uint64_t castlingKey[16];
extern uint64_t epKey[8]; // file of ep square

void getKeys();