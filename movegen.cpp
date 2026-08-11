#include "movegen.h"
#include "board.h"

#include <cstdint>
#include <immintrin.h>


int moveGen(Board& board, move* moves) {
    int cnt = 0;
    bitboard moveMask, mask, pieces;
    int sq;
    int us = board.sideToMove;

    // pawn moves
    if (board.sideToMove == white) {
        // pawn push
        moveMask = (board.pieces[P] << 8) & ~board.occupancies[both] & ~Rank8;
        addMoves(moves, cnt, moveMask, -8);
        // double pawn push
        moveMask = ((((board.pieces[P] & Rank2) << 8) & ~board.occupancies[both]) << 8) & ~board.occupancies[both];
        addMoves(moves, cnt, moveMask, -16, 0b0001);
        // left capture
        moveMask = ((board.pieces[P] & ~FileA) << 7) & board.occupancies[black] & ~Rank8;
        addMoves(moves, cnt, moveMask, -7, 0b0100);
        // right capture
        moveMask = ((board.pieces[P] & ~FileH) << 9) & board.occupancies[black] & ~Rank8;
        addMoves(moves, cnt, moveMask, -9, 0b0100);
        // ep left capture
        if (board.enPassantSquare != 0) {
            moveMask = ((board.pieces[P] & ~FileA) << 7) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, cnt, moveMask, -7, 0b0101);
        // ep right capture
        if (board.enPassantSquare != 0) {
            moveMask = ((board.pieces[P] & ~FileH) << 9) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, cnt, moveMask, -9, 0b0101);
        // pawn push promotion
        moveMask = (board.pieces[P] << 8) & ~board.occupancies[both] & Rank8;
        addPushPromotions(moves, cnt, moveMask, -8);
        // left capture promotion
        moveMask = ((board.pieces[P] & ~FileA) << 7) & board.occupancies[black] & Rank8;
        addCapturePromotions(moves, cnt, moveMask, -7);
        // right capture promotion
        moveMask = ((board.pieces[P] & ~FileH) << 9) & board.occupancies[black] & Rank8;
        addCapturePromotions(moves, cnt, moveMask, -9);
    }
    else {
        // pawn push
        moveMask = (board.pieces[p] >> 8) & ~board.occupancies[both] & ~Rank1;
        addMoves(moves, cnt, moveMask, 8);
        // double pawn push
        moveMask = ((((board.pieces[p] & Rank7) >> 8) & ~board.occupancies[both]) >> 8) & ~board.occupancies[both];
        addMoves(moves, cnt, moveMask, 16, 0b0001);
        // left capture
        moveMask = ((board.pieces[p] & ~FileH) >> 7) & board.occupancies[white] & ~Rank1;
        addMoves(moves, cnt, moveMask, 7, 0b0100);
        // right capture
        moveMask = ((board.pieces[p] & ~FileA) >> 9) & board.occupancies[white] & ~Rank1;
        addMoves(moves, cnt, moveMask, 9, 0b0100);
        // ep left capture
        if (board.enPassantSquare != 0) {
            moveMask = ((board.pieces[p] & ~FileH) >> 7) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, cnt, moveMask, 7, 0b0101);
        // ep right capture
        if (board.enPassantSquare != 0) {
            moveMask = ((board.pieces[p] & ~FileA) >> 9) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, cnt, moveMask, 9, 0b0101);
        // pawn push promotion
        moveMask = (board.pieces[p] >> 8) & ~board.occupancies[both] & Rank1;
        addPushPromotions(moves, cnt, moveMask, 8);
        // left capture promotion
        moveMask = ((board.pieces[p] & ~FileH) >> 7) & board.occupancies[white] & Rank1;
        addCapturePromotions(moves, cnt, moveMask, 7);
        // right capture promotion
        moveMask = ((board.pieces[p] & ~FileA) >> 9) & board.occupancies[white] & Rank1;
        addCapturePromotions(moves, cnt, moveMask, 9);
    }

    // king moves
    int kingSq = __builtin_ctzll(board.pieces[us*6 + K]);
    uint64_t attacks = kingLookup[kingSq];
    addMovesFromAttackMask(moves, cnt, attacks & ~board.occupancies[both], kingSq);
    addMovesFromAttackMask(moves, cnt, attacks & board.occupancies[us^1], kingSq, 0b0100);

    // castling
    if (board.sideToMove == white) {
        if ((board.castling & 0b1000) != 0) {
            if ((~board.occupancies[both] & 0x60) == 0x60) {
                board.sideToMove ^= 1;
                if (isKingSafe(board, 4) && isKingSafe(board, 5) && isKingSafe(board, 6)) {
                    moves[cnt++] = ((0b10 << 12) | (6 << 6) | 4);
                }
                board.sideToMove ^= 1;
            }
        }
        if ((board.castling & 0b0100) != 0) {
            if ((~board.occupancies[both] & 0xe) == 0xe) {
                board.sideToMove ^= 1;
                if (isKingSafe(board, 4) && isKingSafe(board, 3) && isKingSafe(board, 2)) {
                    moves[cnt++] = ((0b11 << 12) | (2 << 6) | 4);
                }
                board.sideToMove ^= 1;
            }
        }
    }
    else {
        if ((board.castling & 0b0010) != 0) {
            if ((~board.occupancies[both] & 0x6000000000000000ULL) == 0x6000000000000000ULL) {
                board.sideToMove ^= 1;
                if (isKingSafe(board, 60) && isKingSafe(board, 61) && isKingSafe(board, 62)) {
                    moves[cnt++] = ((0b10 << 12) | (62 << 6) | 60);
                }
                board.sideToMove ^= 1;
            }
        }
        if ((board.castling & 0b0001) != 0) {
            if ((~board.occupancies[both] & 0xe00000000000000ULL) == 0xe00000000000000ULL) {
                board.sideToMove ^= 1;
                if (isKingSafe(board, 60) && isKingSafe(board, 59) && isKingSafe(board, 58)) {
                    moves[cnt++] = ((0b11 << 12) | (58 << 6) | 60);
                }
                board.sideToMove ^= 1;
            }
        }
    }

    // knight moves
    moveMask = board.pieces[us*6 + N];
    while (moveMask) {
        int knightSq = popLSB(moveMask);
        uint64_t attacks = knightLookup[knightSq];
        addMovesFromAttackMask(moves, cnt, attacks & ~board.occupancies[both], knightSq);
        addMovesFromAttackMask(moves, cnt, attacks & board.occupancies[us^1], knightSq, 0b0100);
    }

    // rook + queen moves
    moveMask = board.pieces[us*6 + R] | board.pieces[us*6 + Q];
    while (moveMask) {
        sq = popLSB(moveMask);
        mask = rookLookup[sq];
        pieces = _pext_u64(board.occupancies[both], mask);
        mask = rookMagic[sq][pieces];
        addMovesFromAttackMask(moves, cnt, mask & ~board.occupancies[both], sq);
        addMovesFromAttackMask(moves, cnt, mask & board.occupancies[us^1], sq, 0b0100);
    }

    // bishop + queen moves
    moveMask = board.pieces[us*6 + B] | board.pieces[us*6 + Q];
    while (moveMask) {
        sq = popLSB(moveMask);
        mask = bishopLookup[sq];
        pieces = _pext_u64(board.occupancies[both], mask);
        mask = bishopMagic[sq][pieces];
        addMovesFromAttackMask(moves, cnt, mask & ~board.occupancies[both], sq);
        addMovesFromAttackMask(moves, cnt, mask & board.occupancies[us^1], sq, 0b0100);
    }

    return cnt;
}