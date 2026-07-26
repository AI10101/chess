#pragma once

#include "constants.h"

#include "string"


class Board {
  public:
    char board[64];
    bool whiteToMove;
    bool cK, cQ, ck, cq; // castling
    int enPassantSquare;
    int halfmoves, fullmoves;

    bitboard pieces[12]; // k, q, b, n, r, p, K, Q, B, N, R, P
    bitboard colour[2]; // white, black
    bitboard empty;
};


void getExtras(Board& board);

void loadFEN(const std::string& s, Board& board);

void makeMove(const uint16_t& move, Board& board);

bool isKingSafe(Board& board);

bitboard getDangerSquares(Board& board);