#include "movegen.h"
#include "board.h"
#include "utility.h"

#include <cstdint>


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

    bitboard moveMask;
    int from, to, flags, i;

    // pawn moves
    if (board.whiteToMove) {
        // pawn push
        moveMask = (board.P << 8) & board.empty & ~Rank8;
        addMoves(moves, moveMask, -8);
        // double pawn push
        moveMask = ((((board.P & Rank2) << 8) & board.empty) << 8) & board.empty;
        addMoves(moves, moveMask, -16, 0b0001);
        // left capture
        moveMask = ((board.P & ~FileA) << 7) & board.black & ~Rank8;
        addMoves(moves, moveMask, -7, 0b0100);
        // right capture
        moveMask = ((board.P & ~FileH) << 9) & board.black & ~Rank8;
        addMoves(moves, moveMask, -9, 0b0100);
        // ep left capture
        if (board.enPassantSquare != -1) {
            moveMask = ((board.P & ~FileA) << 7) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, moveMask, -7, 0b0101);
        // ep right capture
        if (board.enPassantSquare != -1) {
            moveMask = ((board.P & ~FileH) << 9) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, moveMask, -9, 0b0101);
        // pawn push promotion
        moveMask = (board.P << 8) & board.empty & Rank8;
        addPushPromotions(moves, moveMask, -8);
        // left capture promotion
        moveMask = ((board.P & ~FileA) << 7) & board.black & Rank8;
        addCapturePromotions(moves, moveMask, -7);
        // right capture promotion
        moveMask = ((board.P & ~FileH) << 9) & board.black & Rank8;
        addCapturePromotions(moves, moveMask, -9);
    }
    else {
        // pawn push
        moveMask = (board.p >> 8) & board.empty & ~Rank1;
        addMoves(moves, moveMask, 8);
        // double pawn push
        moveMask = ((((board.p & Rank7) >> 8) & board.empty) >> 8) & board.empty;
        addMoves(moves, moveMask, 16, 0b0001);
        // left capture
        moveMask = ((board.p & ~FileH) >> 7) & board.white & ~Rank1;
        addMoves(moves, moveMask, 7, 0b0100);
        // right capture
        moveMask = ((board.p & ~FileA) >> 9) & board.white & ~Rank1;
        addMoves(moves, moveMask, 9, 0b0100);
        // ep left capture
        if (board.enPassantSquare != -1) {
            moveMask = ((board.p & ~FileH) >> 7) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, moveMask, 7, 0b0101);
        // ep right capture
        if (board.enPassantSquare != -1) {
            moveMask = ((board.p & ~FileA) >> 9) & (1ULL << board.enPassantSquare);
        } else moveMask = 0;
        addMoves(moves, moveMask, 9, 0b0101);
        // pawn push promotion
        moveMask = (board.p >> 8) & board.empty & Rank1;
        addPushPromotions(moves, moveMask, 8);
        // left capture promotion
        moveMask = ((board.p & ~FileH) >> 7) & board.white & Rank1;
        addCapturePromotions(moves, moveMask, 7);
        // right capture promotion
        moveMask = ((board.p & ~FileA) >> 9) & board.white & Rank1;
        addCapturePromotions(moves, moveMask, 9);
    }

    // king moves
    if (board.whiteToMove) {
        int kingSq = __builtin_ctzll(board.K);
        uint64_t attacks = kingLookup[kingSq];
        addMovesFromAttackMask(moves, attacks & board.empty, kingSq);
        addMovesFromAttackMask(moves, attacks & board.black, kingSq, 0b0100);
    }
    else {
        int kingSq = __builtin_ctzll(board.k);
        uint64_t attacks = kingLookup[kingSq];
        addMovesFromAttackMask(moves, attacks & board.empty, kingSq);
        addMovesFromAttackMask(moves, attacks & board.white, kingSq, 0b0100);
    }

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
    if (board.whiteToMove) {
        moveMask = board.N;
        while (moveMask) {
            int knightSq = popLSB(moveMask);
            uint64_t attacks = knightLookup[knightSq];
            addMovesFromAttackMask(moves, attacks & board.empty, knightSq);
            addMovesFromAttackMask(moves, attacks & board.black, knightSq, 0b0100);
        }
    }
    else {
        moveMask = board.n;
        while (moveMask) {
            int knightSq = popLSB(moveMask);
            uint64_t attacks = knightLookup[knightSq];
            addMovesFromAttackMask(moves, attacks & board.empty, knightSq);
            addMovesFromAttackMask(moves, attacks & board.white, knightSq, 0b0100);
        }
    }

    // rook + queen moves
    if (board.whiteToMove) {
        // right
        moveMask = (board.R | board.Q) & ~FileH;
        i = 0;
        while (moveMask) {
            moveMask <<= 1;
            i++;
            addMoves(moves, moveMask & board.empty, -i);
            addMoves(moves, moveMask & board.black, -i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~FileH;
        }
        // left
        moveMask = (board.R | board.Q) & ~FileA;
        i = 0;
        while (moveMask) {
            moveMask >>= 1;
            i++;
            addMoves(moves, moveMask & board.empty, i);
            addMoves(moves, moveMask & board.black, i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~FileA;
        }
        // up
        moveMask = (board.R | board.Q) & ~Rank8;
        i = 0;
        while (moveMask) {
            moveMask <<= 8;
            i += 8;
            addMoves(moves, moveMask & board.empty, -i);
            addMoves(moves, moveMask & board.black, -i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~Rank8;
        }
        // down
        moveMask = (board.R | board.Q) & ~Rank1;
        i = 0;
        while (moveMask) {
            moveMask >>= 8;
            i += 8;
            addMoves(moves, moveMask & board.empty, i);
            addMoves(moves, moveMask & board.black, i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~Rank1;
        }
    }
    else {
        // right
        moveMask = (board.r | board.q) & ~FileH;
        i = 0;
        while (moveMask) {
            moveMask <<= 1;
            i++;
            addMoves(moves, moveMask & board.empty, -i);
            addMoves(moves, moveMask & board.white, -i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~FileH;
        }
        // left
        moveMask = (board.r | board.q) & ~FileA;
        i = 0;
        while (moveMask) {
            moveMask >>= 1;
            i++;
            addMoves(moves, moveMask & board.empty, i);
            addMoves(moves, moveMask & board.white, i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~FileA;
        }
        // up
        moveMask = (board.r | board.q) & ~Rank8;
        i = 0;
        while (moveMask) {
            moveMask <<= 8;
            i += 8;
            addMoves(moves, moveMask & board.empty, -i);
            addMoves(moves, moveMask & board.white, -i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~Rank8;
        }
        // down
        moveMask = (board.r | board.q) & ~Rank1;
        i = 0;
        while (moveMask) {
            moveMask >>= 8;
            i += 8;
            addMoves(moves, moveMask & board.empty, i);
            addMoves(moves, moveMask & board.white, i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~Rank1;
        }
    }

    // bishop + queen moves
    if (board.whiteToMove) {
        // up-right
        moveMask = (board.B | board.Q) & ~(FileH | Rank8);
        i = 0;
        while (moveMask) {
            moveMask <<= 9;
            i += 9;
            addMoves(moves, moveMask & board.empty, -i);
            addMoves(moves, moveMask & board.black, -i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~(FileH | Rank8);
        }
        // up-left
        moveMask = (board.B | board.Q) & ~(FileA | Rank8);
        i = 0;
        while (moveMask) {
            moveMask <<= 7;
            i += 7;
            addMoves(moves, moveMask & board.empty, -i);
            addMoves(moves, moveMask & board.black, -i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~(FileA | Rank8);
        }
        // down-right
        moveMask = (board.B | board.Q) & ~(FileH | Rank1);
        i = 0;
        while (moveMask) {
            moveMask >>= 7;
            i += 7;
            addMoves(moves, moveMask & board.empty, i);
            addMoves(moves, moveMask & board.black, i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~(FileH | Rank1);
        }
        // down-left
        moveMask = (board.B | board.Q) & ~(FileA | Rank1);
        i = 0;
        while (moveMask) {
            moveMask >>= 9;
            i += 9;
            addMoves(moves, moveMask & board.empty, i);
            addMoves(moves, moveMask & board.black, i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~(FileA | Rank1);
        }
    }
    else {
        // up-right
        moveMask = (board.b | board.q) & ~(FileH | Rank8);
        i = 0;
        while (moveMask) {
            moveMask <<= 9;
            i += 9;
            addMoves(moves, moveMask & board.empty, -i);
            addMoves(moves, moveMask & board.white, -i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~(FileH | Rank8);
        }
        // up-left
        moveMask = (board.b | board.q) & ~(FileA | Rank8);
        i = 0;
        while (moveMask) {
            moveMask <<= 7;
            i += 7;
            addMoves(moves, moveMask & board.empty, -i);
            addMoves(moves, moveMask & board.white, -i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~(FileA | Rank8);
        }
        // down-right
        moveMask = (board.b | board.q) & ~(FileH | Rank1);
        i = 0;
        while (moveMask) {
            moveMask >>= 7;
            i += 7;
            addMoves(moves, moveMask & board.empty, i);
            addMoves(moves, moveMask & board.white, i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~(FileH | Rank1);
        }
        // down-left
        moveMask = (board.b | board.q) & ~(FileA | Rank1);
        i = 0;
        while (moveMask) {
            moveMask >>= 9;
            i += 9;
            addMoves(moves, moveMask & board.empty, i);
            addMoves(moves, moveMask & board.white, i, 0b0100);
            moveMask &= board.empty;
            moveMask &= ~(FileA | Rank1);
        }
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
            if ((getDangerSquares(next) & next.K) == 0) {
                legalMoves.push_back(m);
            }
        }
        else {
            if ((getDangerSquares(next) & next.k) == 0) {
                legalMoves.push_back(m);
            }
        }
    }

    return legalMoves;
}