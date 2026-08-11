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
    uint8_t board[64]; // not a source of truth - no empty squares handling
    bool sideToMove;
    uint8_t castling; // cK, cQ, ck, cq;
    uint8_t enPassantSquare; // 0 if ep is not possible - ep can not happen on sq 0
    uint16_t halfmoves, fullmoves;

    bitboard pieces[12]; // N, B, R, Q, K, P, n, b, r, q, k, p
    bitboard occupancies[3]; // white, black, both
};


void loadFEN(const std::string& s, Board& board);

void makeMove(const uint16_t move, Board& board);

bool isKingSafe(const Board& board, const int sq);