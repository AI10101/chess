#pragma once

#include <cstdint>


using bitboard = uint64_t;
using move = uint16_t;

extern const bitboard Rank1;
extern const bitboard Rank2;
extern const bitboard Rank7;
extern const bitboard Rank8;

extern const bitboard FileA;
extern const bitboard FileB;
extern const bitboard FileG;
extern const bitboard FileH;

extern bitboard kingLookup[64];
extern bitboard knightLookup[64];
extern bitboard rookLookup[64];
extern bitboard bishopLookup[64];

void getKingLookup();
void getKnightLookup();
void getRookLookup();
void getBishopLookup();

extern bitboard rookMagic[64][4096];
extern bitboard bishopMagic[64][512];

void getRookMagic();
void getBishopMagic();
