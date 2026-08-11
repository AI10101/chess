#pragma once

#include "constants.h"

#include <string>


enum Piece {
    N, B, R, Q, K, P,
    n, b, r, q, k, p
};
enum Occupancy {
    white, black, both
};


class Board {
  public:
    uint8_t board[64];
    bool sideToMove;
    uint8_t castling; // cK, cQ, ck, cq;
    int enPassantSquare;
    int halfmoves, fullmoves;

    bitboard pieces[12]; // N, B, R, Q, K, P, n, b, r, q, k, p
    bitboard occupancies[3]; // white, black, both
};


inline void getExtras(Board& board) {
    board.occupancies[white] = board.pieces[N] | board.pieces[B] | board.pieces[R] | board.pieces[Q] | board.pieces[K] | board.pieces[P];
    board.occupancies[black] = board.pieces[n] | board.pieces[b] | board.pieces[r] | board.pieces[q] | board.pieces[k] | board.pieces[p];
    board.occupancies[both] = board.occupancies[white] | board.occupancies[black];
}


void loadFEN(const std::string& s, Board& board);

void makeMove(const uint16_t move, Board& board);

bool isKingSafe(const Board& board, const int sq);