#pragma once

#include "constants.h"
#include "board.h"
#include "board.h"

#include <cstdint>
#include <immintrin.h>
#include <vector>


inline int popLSB(bitboard& bb) {
    int sq = __builtin_ctzll(bb);
    bb &= bb - 1;
    return sq;
}


template<int Offset>
inline void addPawnMoves(move* moves, int& cnt, bitboard moveMask, const int flags = 0b0000) {
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = ((flags << 12) | (sq << 6) | (sq + Offset));
    }
}


inline void addMovesFromAttackMask(move* moves, int& cnt, bitboard moveMask, const int initialSq, const int flags = 0b0000) {
    uint16_t movePart = (flags << 12) | initialSq;
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = movePart | (sq << 6);
    }
}


template<int Offset, bool Capture>
inline void addPromotions(move* moves, int& cnt, bitboard moveMask) {
    uint16_t movePart = Capture ? (0b11 << 14) : (0b10 << 14);
    while (moveMask) {
        int sq = popLSB(moveMask);
        moves[cnt++] = movePart | (0b00 << 12) | (sq << 6) | (sq + Offset);
        moves[cnt++] = movePart | (0b01 << 12) | (sq << 6) | (sq + Offset);
        moves[cnt++] = movePart | (0b10 << 12) | (sq << 6) | (sq + Offset);
        moves[cnt++] = movePart | (0b11 << 12) | (sq << 6) | (sq + Offset);
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
    piecesToMove = pawnShift<SideToMove, 8>(pawns) & empty;
    addPawnMoves<(SideToMove == white) ? -8 : 8>(moves, cnt, piecesToMove & ~promotionRank);
    addPromotions<(SideToMove == white) ? -8 : 8, false>(moves, cnt, piecesToMove & promotionRank);
    // double push
    piecesToMove = pawnShift<SideToMove, 8>(pawnShift<SideToMove, 8>(pawns & startRank) & empty) & empty;
    addPawnMoves<(SideToMove == white) ? -16 : 16>(moves, cnt, piecesToMove, 0b0001);
    // left capture
    piecesToMove = pawnShift<SideToMove, 7>(pawns & ~leftMostFile);
    addPawnMoves<(SideToMove == white) ? -7 : 7>(moves, cnt, piecesToMove & enemy & ~promotionRank, 0b0100);
    addPromotions<(SideToMove == white) ? -7 : 7, true>(moves, cnt, piecesToMove & enemy & promotionRank); // promotion
    addPawnMoves<(SideToMove == white) ? -7 : 7>(moves, cnt, piecesToMove & board.enPassant, 0b0101); // ep
    // right capture
    piecesToMove = pawnShift<SideToMove, 9>(pawns & ~rightMostFile);
    addPawnMoves<(SideToMove == white) ? -9 : 9>(moves, cnt, piecesToMove & enemy & ~promotionRank, 0b0100);
    addPromotions<(SideToMove == white) ? -9 : 9, true>(moves, cnt, piecesToMove & enemy & promotionRank); // promotion
    addPawnMoves<(SideToMove == white) ? -9 : 9>(moves, cnt, piecesToMove & board.enPassant, 0b0101); // ep
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
        attacks = rookMagic[sq][_pext_u64(occupied, rookLookup[sq])];
        addMovesFromAttackMask(moves, cnt, attacks & empty, sq); // quiet
        addMovesFromAttackMask(moves, cnt, attacks & enemy, sq, 0b0100); // captures
    }

    // bishop + queen moves
    piecesToMove = board.pieces[SideToMove*6 + B] | board.pieces[SideToMove*6 + Q];
    while (piecesToMove) {
        sq = popLSB(piecesToMove);
        attacks = bishopMagic[sq][_pext_u64(occupied, bishopLookup[sq])];
        addMovesFromAttackMask(moves, cnt, attacks & empty, sq); // quiet
        addMovesFromAttackMask(moves, cnt, attacks & enemy, sq, 0b0100); // captures
    }

    return cnt;
}