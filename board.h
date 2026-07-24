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

    bitboard K, Q, B, N, R, P; // white pieces
    bitboard k, q, b, n, r, p; // black pieces
    bitboard white, black, empty;
};


void getExtras(Board& board);

void loadFEN(const std::string& s, Board& board);

void makeMove(const uint16_t& move, Board& board);

bitboard getDangerSquares(Board& board);