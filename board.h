#pragma once

#include "constants.h"

#include <cstdint>
#include <immintrin.h>
#include <string>
#include <sys/cdefs.h>


enum Piece {
    N, B, R, Q, K, P,
    n, b, r, q, k, p
};


enum Occupancy {
    white, black, both
};


struct Undo {
    uint64_t enPassant;
    uint16_t halfmoves;
    uint16_t lastIrreversibleMovePly;
    uint8_t castling;
    uint8_t capturedPiece;
};


class Board {
  public:
    uint8_t board[64]; // not a source of truth - no empty squares handling
    bool sideToMove;
    uint8_t castling; // cK, cQ, ck, cq;
    bitboard enPassant; // 1 where pawn can move to ep capture
    uint16_t halfmoves, fullmoves;

    bitboard pieces[12]; // N, B, R, Q, K, P, n, b, r, q, k, p
    bitboard occupancies[3]; // white, black, both

    uint64_t hashStack[17697];
    Undo undoStack[17697];
    uint16_t ply = 0;
    uint16_t lastIrreversibleMovePly = 0;
};


void loadFEN(const std::string& s, Board& board);


template<bool SideToMove, bool kingCastle>
inline void makeCastlingMove(Board& board, uint64_t& hash) {
    board.halfmoves++;
    board.lastIrreversibleMovePly = board.ply;

    constexpr uint8_t king = (SideToMove) ? k : K;
    constexpr uint8_t rook = (SideToMove) ? r : R;

    constexpr int kingStart = (SideToMove) ? 60 : 4;
    constexpr int kingStop = SideToMove ? (kingCastle ? 62 : 58) : (kingCastle ? 6 : 2);

    constexpr int rookStart = SideToMove ? (kingCastle ? 63 : 56) : (kingCastle ? 7 : 0);
    constexpr int rookStop = SideToMove ? (kingCastle ? 61 : 59) : (kingCastle ? 5 : 3);

    // move king
    board.pieces[king] ^= (1ULL << kingStart) ^ (1ULL << kingStop);
    board.board[kingStop] = king;
    hash ^= pieceSqKey[king][kingStart] ^ pieceSqKey[king][kingStop];
    // move rook 
    board.pieces[rook] ^= (1ULL << rookStart) ^ (1ULL << rookStop);
    board.board[rookStop] = rook;
    hash ^= pieceSqKey[rook][rookStart] ^ pieceSqKey[rook][rookStop];
    // update occupancy bitboards (king + rook)
    board.occupancies[SideToMove] ^= (1ULL << kingStart) ^ (1ULL << kingStop) ^ (1ULL << rookStart) ^ (1ULL << rookStop);
    board.occupancies[both] ^= (1ULL << kingStart) ^ (1ULL << kingStop) ^ (1ULL << rookStart) ^ (1ULL << rookStop);
    // update castling rights
    board.castling &= castlingUpdate[kingStart];
}


template<bool SideToMove>
void makeMove(const uint16_t move, Board& board) {
    int from = move & 0b111111;
    int to = (move >> 6) & 0b111111;
    int flags = (move >> 12) & 0b1111;

    bitboard fromMask = 1ULL << from;
    bitboard toMask = 1ULL << to;
    bitboard fromToMask = fromMask | toMask;

    uint8_t movingPiece = board.board[from];

    board.undoStack[board.ply].enPassant = board.enPassant;
    board.undoStack[board.ply].halfmoves = board.halfmoves;
    board.undoStack[board.ply].lastIrreversibleMovePly = board.lastIrreversibleMovePly; 
    board.undoStack[board.ply].castling = board.castling;

    uint64_t hash = board.hashStack[board.ply];
    if (board.enPassant != 0) hash ^= epKey[__builtin_ctzll(board.enPassant) % 8];

    board.enPassant = 0ULL;

    switch (flags) {
        case 0: // quiet
            // move piece
            board.pieces[movingPiece] ^= fromToMask;
            board.board[to] = movingPiece;
            hash ^= pieceSqKey[movingPiece][from] ^ pieceSqKey[movingPiece][to];
            // update occupancy bitboards
            board.occupancies[SideToMove] ^= fromToMask;
            board.occupancies[both] ^= fromToMask;
            // update halfmove clock
            board.halfmoves++;
            if constexpr (SideToMove) {
                if (movingPiece == p) {
                    board.halfmoves = 0;
                    board.lastIrreversibleMovePly = board.ply;
                }
            }
            else {
                if (movingPiece == P) {
                    board.halfmoves = 0;
                    board.lastIrreversibleMovePly = board.ply;
                }
            }
            // update castling rights
            hash ^= castlingKey[board.castling];
            board.castling &= castlingUpdate[from];
            hash ^= castlingKey[board.castling];
            break;
        case 1: // double pawn push
            board.halfmoves = 0;
            board.lastIrreversibleMovePly = board.ply;
            // move piece
            board.pieces[movingPiece] ^= fromToMask;
            board.board[to] = movingPiece;
            hash ^= pieceSqKey[movingPiece][from] ^ pieceSqKey[movingPiece][to];
            // update occupancy bitboards
            board.occupancies[SideToMove] ^= fromToMask;
            board.occupancies[both] ^= fromToMask;
            // store ep square
            board.enPassant = 1ULL << ((from + to) / 2); 
            hash ^= epKey[to % 8];
            break;
        case 2: // king castle
            makeCastlingMove<SideToMove, true>(board, hash);
            break;
        case 3: // queen castle
            makeCastlingMove<SideToMove, false>(board, hash);
            break;
        case 4: // captures
            board.halfmoves = 0;
            board.lastIrreversibleMovePly = board.ply;
            // remove piece
            board.pieces[board.board[to]] ^= toMask;
            hash ^= pieceSqKey[board.board[to]][to];
            board.undoStack[board.ply].capturedPiece = board.board[to]; // save for undo
            // move attacker
            board.pieces[movingPiece] ^= fromToMask;
            board.board[to] = movingPiece;
            hash ^= pieceSqKey[movingPiece][from] ^ pieceSqKey[movingPiece][to];
            // update occupancy bitboards
            board.occupancies[SideToMove] ^= fromToMask; // us
            board.occupancies[SideToMove^1] ^= toMask; // enemy
            board.occupancies[both] ^= fromMask;
            // update castling rights
            hash ^= castlingKey[board.castling];
            board.castling &= castlingUpdate[from]; // rook or king can capture
            board.castling &= castlingUpdate[to]; // rook can be captured
            hash ^= castlingKey[board.castling];
            break;
        case 5: // ep-capture
            board.halfmoves = 0;
            board.lastIrreversibleMovePly = board.ply;
            // move pawn
            board.pieces[movingPiece] ^= fromToMask;
            board.board[to] = movingPiece;
            hash ^= pieceSqKey[movingPiece][from] ^ pieceSqKey[movingPiece][to];
            if constexpr (SideToMove == white) {
                board.undoStack[board.ply].capturedPiece = p; // save for undo
                // remove pawn
                board.pieces[p] ^= (toMask >> 8);
                hash ^= pieceSqKey[p][to-8];
                // update occupancies
                board.occupancies[white] ^= fromToMask;
                board.occupancies[black] ^= (toMask >> 8);
                board.occupancies[both] ^= fromToMask ^ (toMask >> 8);
            }
            else {
                board.undoStack[board.ply].capturedPiece = P; // save for undo
                // remove pawn
                board.pieces[P] ^= (toMask << 8);
                hash ^= pieceSqKey[P][to+8];
                // update occupancies
                board.occupancies[black] ^= fromToMask;
                board.occupancies[white] ^= (toMask << 8);
                board.occupancies[both] ^= fromToMask ^ (toMask << 8);
            }
            break;
        default: // promotions
            board.halfmoves = 0;
            board.lastIrreversibleMovePly = board.ply;
            if (flags & 0b100) { // capture
                // remove captured piece
                board.pieces[board.board[to]] ^= toMask;
                hash ^= pieceSqKey[board.board[to]][to];
                board.undoStack[board.ply].capturedPiece = board.board[to]; // save for undo 
                board.occupancies[SideToMove^1] ^= toMask; // update enemy occupancy bitboard
                // update castling rights (rook can be captured)
                hash ^= castlingKey[board.castling];
                board.castling &= castlingUpdate[to]; 
                hash ^= castlingKey[board.castling];
            }
            // remove pawn
            board.pieces[movingPiece] ^= fromMask;
            hash ^= pieceSqKey[movingPiece][from];
            // place new piece
            board.board[to] = SideToMove*6 + (flags & 0b11); // flags contain info about promotion piece
            board.pieces[SideToMove*6 + (flags & 0b11)] ^= toMask;
            hash ^= pieceSqKey[SideToMove*6 + (flags & 0b11)][to];
            // update occupancy bitboards
            board.occupancies[SideToMove] ^= fromToMask;
            board.occupancies[both] ^= fromMask;
            board.occupancies[both] |= toMask; // piece could have been capture so nothing changes
            break;
    }

    if constexpr (SideToMove == black) board.fullmoves++;
    board.ply++;

    board.sideToMove ^= 1; // flip side to move
    hash ^= sideToMoveKey;

    board.hashStack[board.ply] = hash;
}


template<bool SideToMove, bool kingCastle>
inline void unmakeCastlingMove(Board& board) {
    constexpr uint8_t king = (SideToMove) ? k : K;
    constexpr uint8_t rook = (SideToMove) ? r : R;

    constexpr int kingStart = (SideToMove) ? 60 : 4;
    constexpr int kingStop = SideToMove ? (kingCastle ? 62 : 58) : (kingCastle ? 6 : 2);

    constexpr int rookStart = SideToMove ? (kingCastle ? 63 : 56) : (kingCastle ? 7 : 0);
    constexpr int rookStop = SideToMove ? (kingCastle ? 61 : 59) : (kingCastle ? 5 : 3);

    // move king
    board.pieces[king] ^= (1ULL << kingStart) ^ (1ULL << kingStop);
    board.board[kingStart] = king;
    // move rook 
    board.pieces[rook] ^= (1ULL << rookStart) ^ (1ULL << rookStop);
    board.board[rookStart] = rook;
    // update occupancy bitboards (king + rook)
    board.occupancies[SideToMove] ^= (1ULL << kingStart) ^ (1ULL << kingStop) ^ (1ULL << rookStart) ^ (1ULL << rookStop);
    board.occupancies[both] ^= (1ULL << kingStart) ^ (1ULL << kingStop) ^ (1ULL << rookStart) ^ (1ULL << rookStop);
}


template<bool SideToMove>
void unmakeMove(const uint16_t move, Board& board) {
    int from = move & 0b111111;
    int to = (move >> 6) & 0b111111;
    int flags = (move >> 12) & 0b1111;

    bitboard fromMask = 1ULL << from;
    bitboard toMask = 1ULL << to;
    bitboard fromToMask = fromMask | toMask;

    uint8_t movingPiece = board.board[to];

    Undo undo = board.undoStack[--board.ply];

    if constexpr (SideToMove == black) board.fullmoves--;

    board.sideToMove ^= 1; // flip side to move

    board.enPassant = undo.enPassant;
    board.halfmoves = undo.halfmoves;
    board.lastIrreversibleMovePly = undo.lastIrreversibleMovePly;
    board.castling = undo.castling;

    switch (flags) {
        case 0: // quiet
        case 1: // double pawn push
            // move piece back
            board.pieces[movingPiece] ^= fromToMask;
            board.board[from] = movingPiece;
            // update occupancy bitboards
            board.occupancies[SideToMove] ^= fromToMask;
            board.occupancies[both] ^= fromToMask;
            break;
        case 2: // king castle
            unmakeCastlingMove<SideToMove, true>(board);
            break;
        case 3: // queen castle
            unmakeCastlingMove<SideToMove, false>(board);
            break;
        case 4: // captures
            // move piece back
            board.pieces[movingPiece] ^= fromToMask;
            board.board[from] = movingPiece;
            // restore captured piece
            board.pieces[undo.capturedPiece] ^= toMask;
            board.board[to] = undo.capturedPiece;
            // update occupancy bitboards
            board.occupancies[SideToMove] ^= fromToMask;
            board.occupancies[SideToMove^1] ^= toMask;
            board.occupancies[both] ^= fromMask;
            break;
        case 5: // ep-capture
            // move pawn back
            board.pieces[movingPiece] ^= fromToMask;
            board.board[from] = movingPiece;
            // update occupancy bitboards
            board.occupancies[SideToMove] ^= fromToMask;
            board.occupancies[both] ^= fromToMask;
            // restore captured pawn
            if constexpr (SideToMove == white) {
                board.pieces[p] ^= toMask >> 8;
                board.board[to-8] = p;
                board.occupancies[black] ^= toMask >> 8;
                board.occupancies[both] ^= toMask >> 8;
            }
            else {
                board.pieces[P] ^= toMask << 8;
                board.board[to+8] = P;
                board.occupancies[white] ^= toMask << 8;
                board.occupancies[both] ^= toMask << 8;
            }
            break;
        default: // promotions
            // remove new piece
            board.pieces[movingPiece] ^= toMask;
            board.occupancies[SideToMove] ^= toMask;
            board.occupancies[both] ^= toMask;
            // restore pawn
            board.occupancies[SideToMove] ^= fromMask;
            board.occupancies[both] ^= fromMask;
            if constexpr (SideToMove == white) {
                board.pieces[P] ^= fromMask;
                board.board[from] = P;
            }
            else {
                board.pieces[p] ^= fromMask;
                board.board[from] = p;
            }
            // restore captured piece
            if (flags & 0b100) {
                board.pieces[undo.capturedPiece] ^= toMask;
                board.board[to] = undo.capturedPiece;
                board.occupancies[SideToMove^1] ^= toMask;
                board.occupancies[both] ^= toMask;
            }
            break;
    }
}


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