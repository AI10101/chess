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
extern bitboard rookLookup[64];
extern bitboard bishopLookup[64];

void getKingLookup();
void getKnightLookup();
void getRookLookup();
void getBishopLookup();

extern bitboard rookMagic[64][16384];
extern bitboard bishopMagic[64][8192];

void getRookMagic();
void getBishopMagic();
