#include "movegen.h"
#include "board.h"
#include "utility.h"

#include <cstdint>
#include <immintrin.h>


void addMoves(std::vector<move>& moves, bitboard moveMask, int offset, uint16_t flags = 0b0000) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves.push_back((flags << 12) | (sq << 6) | (sq + offset));
    }
}

void addMovesFromAttackMask(std::vector<move>& moves, bitboard moveMask, int initialSq, uint16_t flags = 0b0000) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves.push_back((flags << 12) | (sq << 6) | initialSq);
    }
}

void addPushPromotions(std::vector<move>& moves, bitboard moveMask, int offset) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves.push_back((0b1000 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1001 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1010 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1011 << 12) | (sq << 6) | (sq + offset));
    }
}

void addCapturePromotions(std::vector<move>& moves, bitboard moveMask, int offset) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves.push_back((0b1100 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1101 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1110 << 12) | (sq << 6) | (sq + offset));
        moves.push_back((0b1111 << 12) | (sq << 6) | (sq + offset));
    }
}

std::vector<move> moveGen(Board& board) {
    std::vector<move> moves;

    bitboard moveMask, mask, pieces;
    int from, to, flags, i, sq;
    int us = board.whiteToMove;

    // pawn moves
    if (board.whiteToMove) {
        // pawn push
        moveMask = (board.pieces[11] << 8) & board.empty & ~Rank8;
        addMoves(moves, moveMask, -8);
        // double pawn push
        moveMask = ((((board.pieces[11] & Rank2) << 8) & board.empty) << 8) & board.empty;
        addMoves(moves, moveMask, -16, 0b0001);
        // left capture
        moveMask = ((board.pieces[11] & ~FileA) << 7) & board.colour[1] & ~Rank8;
        addMoves(moves, moveMask, -7, 0b0100);
        // right capture
        moveMask = ((board.pieces[11] & ~FileH) << 9) & board.colour[1] & ~Rank8;
        addMoves(moves, moveMask, -9, 0b0100);
        // ep left capture
        if (board.enPassantSquare != -1) {
            moveMask = ((board.pieces[11] & ~FileA) << 7) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, moveMask, -7, 0b0101);
        // ep right capture
        if (board.enPassantSquare != -1) {
            moveMask = ((board.pieces[11] & ~FileH) << 9) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, moveMask, -9, 0b0101);
        // pawn push promotion
        moveMask = (board.pieces[11] << 8) & board.empty & Rank8;
        addPushPromotions(moves, moveMask, -8);
        // left capture promotion
        moveMask = ((board.pieces[11] & ~FileA) << 7) & board.colour[1] & Rank8;
        addCapturePromotions(moves, moveMask, -7);
        // right capture promotion
        moveMask = ((board.pieces[11] & ~FileH) << 9) & board.colour[1] & Rank8;
        addCapturePromotions(moves, moveMask, -9);
    }
    else {
        // pawn push
        moveMask = (board.pieces[5] >> 8) & board.empty & ~Rank1;
        addMoves(moves, moveMask, 8);
        // double pawn push
        moveMask = ((((board.pieces[5] & Rank7) >> 8) & board.empty) >> 8) & board.empty;
        addMoves(moves, moveMask, 16, 0b0001);
        // left capture
        moveMask = ((board.pieces[5] & ~FileH) >> 7) & board.colour[0] & ~Rank1;
        addMoves(moves, moveMask, 7, 0b0100);
        // right capture
        moveMask = ((board.pieces[5] & ~FileA) >> 9) & board.colour[0] & ~Rank1;
        addMoves(moves, moveMask, 9, 0b0100);
        // ep left capture
        if (board.enPassantSquare != -1) {
            moveMask = ((board.pieces[5] & ~FileH) >> 7) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, moveMask, 7, 0b0101);
        // ep right capture
        if (board.enPassantSquare != -1) {
            moveMask = ((board.pieces[5] & ~FileA) >> 9) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, moveMask, 9, 0b0101);
        // pawn push promotion
        moveMask = (board.pieces[5] >> 8) & board.empty & Rank1;
        addPushPromotions(moves, moveMask, 8);
        // left capture promotion
        moveMask = ((board.pieces[5] & ~FileH) >> 7) & board.colour[0] & Rank1;
        addCapturePromotions(moves, moveMask, 7);
        // right capture promotion
        moveMask = ((board.pieces[5] & ~FileA) >> 9) & board.colour[0] & Rank1;
        addCapturePromotions(moves, moveMask, 9);
    }

    // king moves
    int kingSq = __builtin_ctzll(board.pieces[us*6]);
    uint64_t attacks = kingLookup[kingSq];
    addMovesFromAttackMask(moves, attacks & board.empty, kingSq);
    addMovesFromAttackMask(moves, attacks & board.colour[us], kingSq, 0b0100);

    // castling
    if (board.whiteToMove) {
        if (board.cK) {
            if ((board.empty & 0x60) == 0x60) {
                uint64_t danger = getDangerSquares(board);
                if ((danger & 0x70) == 0) {
                    moves.push_back((0b10 << 12) | (6 << 6) | 4);
                }
            }
        }
        if (board.cQ) {
            if ((board.empty & 0xe) == 0xe) {
                uint64_t danger = getDangerSquares(board);
                if ((danger & 0x1c) == 0) {
                    moves.push_back((0b11 << 12) | (2 << 6) | 4);
                }
            }
        }
    }
    else {
        if (board.ck) {
            if ((board.empty & 0x6000000000000000ULL) == 0x6000000000000000ULL) {
                uint64_t danger = getDangerSquares(board);
                if ((danger & 0x7000000000000000ULL) == 0) {
                    moves.push_back((0b10 << 12) | (62 << 6) | 60);
                }
            }
        }
        if (board.cq) {
            if ((board.empty & 0xe00000000000000ULL) == 0xe00000000000000ULL) {
                uint64_t danger = getDangerSquares(board);
                if ((danger & 0x1c00000000000000) == 0) {
                    moves.push_back((0b11 << 12) | (58 << 6) | 60);
                }
            }
        }
    }

    // knight moves
    moveMask = board.pieces[us*6 + 3];
    while (moveMask) {
        int knightSq = popLSB(moveMask);
        uint64_t attacks = knightLookup[knightSq];
        addMovesFromAttackMask(moves, attacks & board.empty, knightSq);
        addMovesFromAttackMask(moves, attacks & board.colour[us], knightSq, 0b0100);
    }

    // rook + queen moves
    moveMask = board.pieces[us*6 + 4] | board.pieces[us*6 + 1];
    while (moveMask) {
        sq = popLSB(moveMask);
        mask = rookLookup[sq];
        pieces = _pext_u64(~board.empty, mask);
        mask = rookMagic[sq][pieces];
        addMovesFromAttackMask(moves, mask & board.empty, sq);
        addMovesFromAttackMask(moves, mask & board.colour[us], sq, 0b0100);
    }

    // bishop + queen moves
    moveMask = board.pieces[us*6 + 2] | board.pieces[us*6 + 1];
    while (moveMask) {
        sq = popLSB(moveMask);
        mask = bishopLookup[sq];
        pieces = _pext_u64(~board.empty, mask);
        mask = bishopMagic[sq][pieces];
        addMovesFromAttackMask(moves, mask & board.empty, sq);
        addMovesFromAttackMask(moves, mask & board.colour[us], sq, 0b0100);
    }

    return moves;
}

std::vector<move> legalMoveGen(Board& board) {
    std::vector<move> pseudoMoves = moveGen(board);

    std::vector<move> legalMoves;

    for (move m : pseudoMoves) {
        Board next = board;
        makeMove(m, next);

        next.whiteToMove = !next.whiteToMove;
        if (next.whiteToMove) {
            if ((getDangerSquares(next) & next.pieces[6]) == 0) {
                legalMoves.push_back(m);
            }
        }
        else {
            if ((getDangerSquares(next) & next.pieces[0]) == 0) {
                legalMoves.push_back(m);
            }
        }
    }

    return legalMoves;
}