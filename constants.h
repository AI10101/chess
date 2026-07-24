#pragma once

#include <cstdint>


using bitboard = uint64_t;
using move = uint16_t;

extern bitboard Rank1;
extern bitboard Rank2;
extern bitboard Rank7;
extern bitboard Rank8;

extern bitboard FileA;
extern bitboard FileB;
extern bitboard FileG;
extern bitboard FileH;

extern bitboard kingLookup[64];
extern bitboard knightLookup[64];

void getKingLookup();
void getKnightLookup();
