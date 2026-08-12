#pragma once

#include "constants.h"
#include "board.h"
#include "board.h"

#include <immintrin.h>
#include <vector>


inline int popLSB(bitboard& bb) {
    int sq = __builtin_ctzll(bb);
    bb &= bb - 1;
    return sq;
}


template<int Offset, int Flags>
inline void addPawnMoves(move* moves, int& cnt, bitboard moveMask) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((Flags << 12) | (sq << 6) | (sq + Offset));
    }
}

inline void addMovesFromAttackMask(move* moves, int& cnt, bitboard moveMask, const int initialSq, const int flags = 0b0000) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((flags << 12) | (sq << 6) | initialSq);
    }
}


template<int Offset>
inline void addPushPromotions(move* moves, int& cnt, bitboard moveMask) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((0b1000 << 12) | (sq << 6) | (sq + Offset));
        moves[cnt++] = ((0b1001 << 12) | (sq << 6) | (sq + Offset));
        moves[cnt++] = ((0b1010 << 12) | (sq << 6) | (sq + Offset));
        moves[cnt++] = ((0b1011 << 12) | (sq << 6) | (sq + Offset));
    }
}


template<int Offset>
inline void addCapturePromotions(move* moves, int& cnt, bitboard moveMask) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((0b1100 << 12) | (sq << 6) | (sq + Offset));
        moves[cnt++] = ((0b1101 << 12) | (sq << 6) | (sq + Offset));
        moves[cnt++] = ((0b1110 << 12) | (sq << 6) | (sq + Offset));
        moves[cnt++] = ((0b1111 << 12) | (sq << 6) | (sq + Offset));
    }
}


template<bool SideToMove, int Shift>
inline bitboard pawnShift(const bitboard pawns) {
    if constexpr (SideToMove == white) {
        return pawns << Shift;
    }
    return pawns >> Shift;
}


template<bool SideToMove>
inline void pawnMoveGen(Board& board, move* moves, int& cnt, const bitboard empty, const bitboard enemy) {
    bitboard piecesToMove;

    constexpr const bitboard promotionRank = (SideToMove == white) ? Rank8 : Rank1;
    constexpr const bitboard startRank = (SideToMove == white) ? Rank2 : Rank7;
    constexpr const bitboard leftMostFile = (SideToMove == white) ? FileA : FileH;
    constexpr const bitboard rightMostFile = (SideToMove == white) ? FileH : FileA;

    const bitboard pawns = board.pieces[(SideToMove == white) ? P : p];

    // push
    piecesToMove = pawnShift<SideToMove, 8>(pawns) & empty & ~promotionRank;
    addPawnMoves<(SideToMove == white) ? -8 : 8, 0b0000>(moves, cnt, piecesToMove);
    // double push
    piecesToMove = pawnShift<SideToMove, 8>(pawnShift<SideToMove, 8>(pawns & startRank) & empty) & empty;
    addPawnMoves<(SideToMove == white) ? -16 : 16, 0b0001>(moves, cnt, piecesToMove);
    // left capture
    piecesToMove = pawnShift<SideToMove, 7>(pawns & ~leftMostFile) & enemy & ~promotionRank;
    addPawnMoves<(SideToMove == white) ? -7 : 7, 0b0100>(moves, cnt, piecesToMove);
    // right capture
    piecesToMove = pawnShift<SideToMove, 9>(pawns & ~rightMostFile) & enemy & ~promotionRank;
    addPawnMoves<(SideToMove == white) ? -9 : 9, 0b0100>(moves, cnt, piecesToMove);
    // ep
    if (board.enPassantSquare != 0) {
        // left
        piecesToMove = pawnShift<SideToMove, 7>(pawns & ~leftMostFile) & (1ULL << board.enPassantSquare);
        addPawnMoves<(SideToMove == white) ? -7 : 7, 0b0101>(moves, cnt, piecesToMove);
        // right
        piecesToMove = pawnShift<SideToMove, 9>(pawns & ~rightMostFile) & (1ULL << board.enPassantSquare);
        addPawnMoves<(SideToMove == white) ? -9 : 9, 0b0101>(moves, cnt, piecesToMove);
    }
    // pawn push promotion
    piecesToMove = pawnShift<SideToMove, 8>(pawns) & empty & promotionRank;
    addPushPromotions<(SideToMove == white) ? -8 : 8>(moves, cnt, piecesToMove);
    // left capture promotion
    piecesToMove = pawnShift<SideToMove, 7>(pawns & ~leftMostFile) & enemy & promotionRank;
    addCapturePromotions<(SideToMove == white) ? -7 : 7>(moves, cnt, piecesToMove);
    // right capture promotion
    piecesToMove = pawnShift<SideToMove, 9>(pawns & ~rightMostFile) & enemy & promotionRank;
    addCapturePromotions<(SideToMove == white) ? -9 : 9>(moves, cnt, piecesToMove);
}


template<bool SideToMove>
int moveGen(Board& board, move* moves) {
    int cnt = 0;
    const bitboard own = board.occupancies[SideToMove], enemy = board.occupancies[SideToMove^1];
    const bitboard occupied = board.occupancies[both], empty = ~occupied;
    int sq;
    bitboard piecesToMove, attacks;

    // pawn moves
    pawnMoveGen<SideToMove>(board, moves, cnt, empty, enemy);

    // castling
    if constexpr (SideToMove == white) {
        if (board.castling & 0b1000) {
            if ((occupied & 0x60) == 0ULL) {
                if (isSqSafe<SideToMove^1>(board, 4) && isSqSafe<SideToMove^1>(board, 5) && isSqSafe<SideToMove^1>(board, 6)) {
                    moves[cnt++] = ((0b10 << 12) | (6 << 6) | 4);
                }
            }
        }
        if (board.castling & 0b0100) {
            if ((occupied & 0xe) == 0ULL) {
                if (isSqSafe<SideToMove^1>(board, 4) && isSqSafe<SideToMove^1>(board, 3) && isSqSafe<SideToMove^1>(board, 2)) {
                    moves[cnt++] = ((0b11 << 12) | (2 << 6) | 4);
                }
            }
        }
    }
    else {
        if (board.castling & 0b0010) {
            if ((occupied & 0x6000000000000000ULL) == 0ULL) {
                if (isSqSafe<SideToMove^1>(board, 60) && isSqSafe<SideToMove^1>(board, 61) && isSqSafe<SideToMove^1>(board, 62)) {
                    moves[cnt++] = ((0b10 << 12) | (62 << 6) | 60);
                }
            }
        }
        if (board.castling & 0b0001) {
            if ((occupied & 0xe00000000000000ULL) == 0ULL) {
                if (isSqSafe<SideToMove^1>(board, 60) && isSqSafe<SideToMove^1>(board, 59) && isSqSafe<SideToMove^1>(board, 58)) {
                    moves[cnt++] = ((0b11 << 12) | (58 << 6) | 60);
                }
            }
        }
    }

    // king moves
    sq = __builtin_ctzll(board.pieces[SideToMove*6 + K]); // there is always exactly 1 king
    attacks = kingLookup[sq];
    addMovesFromAttackMask(moves, cnt, attacks & empty, sq); // quiet
    addMovesFromAttackMask(moves, cnt, attacks & enemy, sq, 0b0100); // captures

    // knight moves
    piecesToMove = board.pieces[SideToMove*6 + N];
    while (piecesToMove) {
        sq = popLSB(piecesToMove);
        attacks = knightLookup[sq];
        addMovesFromAttackMask(moves, cnt, attacks & empty, sq); // quiet
        addMovesFromAttackMask(moves, cnt, attacks & enemy, sq, 0b0100); // captures
    }

    // rook + queen moves
    piecesToMove = board.pieces[SideToMove*6 + R] | board.pieces[SideToMove*6 + Q];
    while (piecesToMove) {
        sq = popLSB(piecesToMove);
        // attacks is used counterintuitively to avoid declaration of new bitboards (better cache access)
        attacks = rookLookup[sq]; // just a mask for extracting pieces blocking movement
        attacks = _pext_u64(occupied, attacks); // extracted pieces blocking movement - index for magic lookup
        attacks = rookMagic[sq][attacks];
        addMovesFromAttackMask(moves, cnt, attacks & empty, sq); // quiet
        addMovesFromAttackMask(moves, cnt, attacks & enemy, sq, 0b0100); // captures
    }

    // bishop + queen moves
    piecesToMove = board.pieces[SideToMove*6 + B] | board.pieces[SideToMove*6 + Q];
    while (piecesToMove) {
        sq = popLSB(piecesToMove);
        // attacks is used counterintuitively to avoid declaration of new bitboards (better cache access)
        attacks = bishopLookup[sq]; // just a mask for extracting pieces blocking movement
        attacks = _pext_u64(occupied, attacks); // extracted pieces blocking movement - index for magic lookup
        attacks = bishopMagic[sq][attacks];
        addMovesFromAttackMask(moves, cnt, attacks & empty, sq); // quiet
        addMovesFromAttackMask(moves, cnt, attacks & enemy, sq, 0b0100); // captures
    }

    return cnt;
}