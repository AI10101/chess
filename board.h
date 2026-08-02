#pragma once

#include "constants.h"

#include "string"


class Board {
  public:
    uint8_t board[64];
    bool whiteToMove;
    bool cK, cQ, ck, cq; // castling
    int enPassantSquare;
    int halfmoves, fullmoves;

    bitboard pieces[12]; // k, q, b, n, r, p, K, Q, B, N, R, P
    bitboard colour[2]; // white, black
    bitboard empty;
};


inline void getExtras(Board& board) {
    board.colour[0] = board.pieces[6] | board.pieces[7] | board.pieces[8] | board.pieces[9] | board.pieces[10] | board.pieces[11];
    board.colour[1] = board.pieces[0] | board.pieces[1] | board.pieces[2] | board.pieces[3] | board.pieces[4] | board.pieces[5];
    board.empty = ~(board.colour[0] | board.colour[1]);
}


void loadFEN(const std::string& s, Board& board);

void makeMove(const uint16_t move, Board& board);

bool isKingSafe(const Board& board, const int sq);