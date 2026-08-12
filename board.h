#pragma once

#include "constants.h"

#include <immintrin.h>
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


template<bool enemy>
inline bool isSqSafe(const Board& board, const int sq) {
    bitboard attackers = 0ULL;
    bitboard mask;

    // we check if sq is attacked by any enemy piece

    // king
    attackers |= kingLookup[sq] & board.pieces[enemy*6 + K];

    // knight
    attackers |= knightLookup[sq] & board.pieces[enemy*6 + N];

    // rook + queen
    mask = rookLookup[sq];
    mask = _pext_u64(board.occupancies[both], mask);
    attackers |= rookMagic[sq][mask] & (board.pieces[enemy*6 + R] | board.pieces[enemy*6 + Q]);

    // bishop + queen
    mask = bishopLookup[sq];
    mask = _pext_u64(board.occupancies[both], mask);
    attackers |= bishopMagic[sq][mask] & (board.pieces[enemy*6 + B] | board.pieces[enemy*6 + Q]);

    mask = 1ULL << sq;

    // pawns
    if constexpr (enemy == white) {
        attackers |= ((board.pieces[P] & ~FileA) << 7) & mask; // left capture
        attackers |= ((board.pieces[P] & ~FileH) << 9) & mask; // right capture
    }
    else {
        // left capture
        attackers |= ((board.pieces[p] & ~FileH) >> 7) & mask; // left capture
        attackers |= ((board.pieces[p] & ~FileA) >> 9) & mask; // right capture
    }

    return (attackers == 0ULL);
}