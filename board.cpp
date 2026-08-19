#include "board.h"
#include "constants.h"
#include "movegen.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <immintrin.h>


void loadFEN(const std::string& s, Board& board) {
    std::stringstream fen(s);

    std::fill(std::begin(board.pieces), std::end(board.pieces), 0ULL);
    uint64_t hash = 0ULL;
    board.ply = 0;

    std::string placement; fen >> placement;
    int rank = 7, file = 0;
    for (char c: placement) {
        if (c == '/') {
            rank--; file = 0;
        }
        else if (isdigit(c)) {
            file += c - '0';
        }
        else {
            int sq = rank * 8 + file;
            file++;
            switch(c) {
                case 'K': board.pieces[K] |= 1ULL << sq; board.board[sq] = K; hash ^= pieceSqKey[K][sq]; break;
                case 'Q': board.pieces[Q] |= 1ULL << sq; board.board[sq] = Q; hash ^= pieceSqKey[Q][sq]; break;
                case 'B': board.pieces[B] |= 1ULL << sq; board.board[sq] = B; hash ^= pieceSqKey[B][sq]; break;
                case 'N': board.pieces[N] |= 1ULL << sq; board.board[sq] = N; hash ^= pieceSqKey[N][sq]; break;
                case 'R': board.pieces[R] |= 1ULL << sq; board.board[sq] = R; hash ^= pieceSqKey[R][sq]; break;
                case 'P': board.pieces[P] |= 1ULL << sq; board.board[sq] = P; hash ^= pieceSqKey[P][sq]; break;

                case 'k': board.pieces[k] |= 1ULL << sq; board.board[sq] = k; hash ^= pieceSqKey[k][sq]; break;
                case 'q': board.pieces[q] |= 1ULL << sq; board.board[sq] = q; hash ^= pieceSqKey[q][sq]; break;
                case 'b': board.pieces[b] |= 1ULL << sq; board.board[sq] = b; hash ^= pieceSqKey[b][sq]; break;
                case 'n': board.pieces[n] |= 1ULL << sq; board.board[sq] = n; hash ^= pieceSqKey[n][sq]; break;
                case 'r': board.pieces[r] |= 1ULL << sq; board.board[sq] = r; hash ^= pieceSqKey[r][sq]; break;
                case 'p': board.pieces[p] |= 1ULL << sq; board.board[sq] = p; hash ^= pieceSqKey[p][sq]; break;
            }
        }
    }

    // construct occupancies
    board.occupancies[white] = board.pieces[N] | board.pieces[B] | board.pieces[R] | board.pieces[Q] | board.pieces[K] | board.pieces[P];
    board.occupancies[black] = board.pieces[n] | board.pieces[b] | board.pieces[r] | board.pieces[q] | board.pieces[k] | board.pieces[p];
    board.occupancies[both] = board.occupancies[white] | board.occupancies[black];

    std::string movingSide; fen >> movingSide;
    board.sideToMove = (movingSide == "b"); // "b" = 1; "w" = 0
    if (board.sideToMove) hash ^= sideToMoveKey;

    std::string castling; fen >> castling;
    board.castling = 0;
    for (char c: castling) {
        switch (c) {
            case 'K': board.castling ^= 0b1000; break;
            case 'Q': board.castling ^= 0b100; break;
            case 'k': board.castling ^= 0b10; break;
            case 'q': board.castling ^= 0b1; break;
        }
    }
    hash ^= castlingKey[board.castling];

    std::string enPassant; fen >> enPassant;
    if (enPassant == "-") {
        board.enPassant = 0ULL;
    }
    else {
        board.enPassant = 1ULL << ((enPassant[1] -'1') * 8 + (enPassant[0] - 'a'));
        hash ^= epKey[enPassant[0] - 'a'];
    }

    board.hashStack[0] = hash;

    std::string movesCnt;
    fen >> movesCnt; board.halfmoves = stoi(movesCnt);
    fen >> movesCnt; board.fullmoves = stoi(movesCnt);
}


uint64_t getHash(Board &board) {
    uint64_t hash = 0ULL;

    for (int piece=0; piece < 12; piece++) {
        bitboard bb = board.pieces[piece];

        while (bb) {
            int sq = popLSB(bb);
            hash ^= pieceSqKey[piece][sq];
        }
    }

    if (board.sideToMove) hash ^= sideToMoveKey;

    hash ^= castlingKey[board.castling];

    if (board.enPassant != 0) {
        hash ^= epKey[__builtin_ctzll(board.enPassant) % 8];
    }

    return hash;
}


void makeMove(const uint16_t move, Board& board) {
    int from = move & 0b111111;
    int to = (move >> 6) & 0b111111;
    int flags = (move >> 12) & 0b1111;

    uint64_t fromMask = 1ULL << from;
    uint64_t toMask = 1ULL << to;

    board.undoStack[board.ply].enPassant = board.enPassant;
    board.undoStack[board.ply].halfmoves = board.halfmoves; 
    board.undoStack[board.ply].castling = board.castling;

    uint64_t hash = board.hashStack[board.ply];

    if (board.enPassant != 0) hash ^= epKey[__builtin_ctzll(board.enPassant) % 8];
    board.enPassant = 0ULL;

    hash ^= castlingKey[board.castling];

    switch (flags) {
        case 0: // quiet
            board.pieces[board.board[from]] ^= fromMask | toMask; // update bitboard
            hash ^= pieceSqKey[board.board[from]][from];
            hash ^= pieceSqKey[board.board[from]][to];
            // update occupancy bitboards
            board.occupancies[board.sideToMove] ^= fromMask | toMask;
            board.occupancies[both] ^= fromMask | toMask;
            // update board
            board.board[to] = board.board[from];
            // update halfmove clock
            if (board.board[to] == p || board.board[to] == P) board.halfmoves = 0;
            else board.halfmoves++;
            // update castling rights
            board.castling &= castlingUpdate[from];
            break;
        case 1: // double pawn push
            board.halfmoves = 0;
            board.pieces[board.board[from]] ^= fromMask | toMask; // update bitboard
            hash ^= pieceSqKey[board.board[from]][from];
            hash ^= pieceSqKey[board.board[from]][to];
            // update occupancy bitboards
            board.occupancies[board.sideToMove] ^= fromMask | toMask;
            board.occupancies[both] ^= fromMask | toMask;
            // update board
            board.board[to] = board.board[from];
            board.enPassant = 1ULL << ((from + to) / 2); // store ep square
            hash ^= epKey[__builtin_ctzll(board.enPassant) % 8];
            break;
        case 2: // king castle
            board.halfmoves++;
            // update bitboards
            board.pieces[board.board[from]] ^= fromMask | toMask; // king
            hash ^= pieceSqKey[board.board[from]][from];
            hash ^= pieceSqKey[board.board[from]][to];
            board.pieces[board.board[from+3]] ^= (toMask >> 1) | (toMask << 1); // rook
            hash ^= pieceSqKey[board.board[from+3]][from+3];
            hash ^= pieceSqKey[board.board[from+3]][from+1];
            // update occupancy bitboards (king + rook)
            board.occupancies[board.sideToMove] ^= fromMask | toMask | (toMask >> 1) | (toMask << 1);
            board.occupancies[both] ^= fromMask | toMask | (toMask >> 1) | (toMask << 1);
            // update board
            board.board[to] = board.board[from]; // king
            board.board[from+1] = board.board[from+3]; // rook
            // update castling rights
            board.castling &= castlingUpdate[from];
            break;
        case 3: // queen castle
            board.halfmoves++;
            // update bitboards
            board.pieces[board.board[from]] ^= fromMask | toMask; // king
            hash ^= pieceSqKey[board.board[from]][from];
            hash ^= pieceSqKey[board.board[from]][to];
            board.pieces[board.board[from-4]] ^= (toMask >> 2) | (toMask << 1); // rook
            hash ^= pieceSqKey[board.board[from-4]][from-4];
            hash ^= pieceSqKey[board.board[from-4]][from-1];
            // update occupancy bitboards (king + rook)
            board.occupancies[board.sideToMove] ^= fromMask | toMask | (toMask >> 2) | (toMask << 1);
            board.occupancies[both] ^= fromMask | toMask | (toMask >> 2) | (toMask << 1);
            // update board
            board.board[to] = board.board[from]; // king
            board.board[from-1] = board.board[from-4]; // rook
            // update castling rights
            board.castling &= castlingUpdate[from];
            break;
        case 4: // captures
            board.halfmoves = 0;
            board.pieces[board.board[from]] ^= fromMask | toMask; // update attacker bitboard
            hash ^= pieceSqKey[board.board[from]][from];
            hash ^= pieceSqKey[board.board[from]][to];
            board.pieces[board.board[to]] ^= toMask; // update captured bitboard
            hash ^= pieceSqKey[board.board[to]][to];
            board.undoStack[board.ply].capturedPiece = board.board[to]; 
            // update occupancy bitboards
            board.occupancies[board.sideToMove] ^= fromMask | toMask; // us
            board.occupancies[board.sideToMove^1] ^= toMask; // enemy
            board.occupancies[both] ^= fromMask;
            // update board
            board.board[to] = board.board[from];
            // update castling rights
            board.castling &= castlingUpdate[from]; // rook or king can capture
            board.castling &= castlingUpdate[to]; // rook can be captured
            break;
        case 5: // ep-capture
            board.halfmoves = 0;
            board.pieces[board.board[from]] ^= fromMask | toMask; // update pawn bitboard
            hash ^= pieceSqKey[board.board[from]][from];
            hash ^= pieceSqKey[board.board[from]][to];
            board.undoStack[board.ply].capturedPiece = board.sideToMove == white ? p : P;; 
            board.board[to] = board.board[from]; // update board
            // the if statement is unnecessary as to+8 and to-8 squares contain precisely one pawn - the captured one
            board.pieces[p] &= ~((toMask << 8) | (toMask >> 8));
            board.pieces[P] &= ~((toMask << 8) | (toMask >> 8));
            if (board.sideToMove == white) {
                hash ^= pieceSqKey[board.board[to-8]][to-8];
            }
            else {
                hash ^= pieceSqKey[board.board[to+8]][to+8];
            }
            // update occupancy bitboards
            board.occupancies[board.sideToMove] ^= fromMask | toMask; // us
            board.occupancies[board.sideToMove^1] &= ~((toMask << 8) | (toMask >> 8)); // enemy
            board.occupancies[both] ^= fromMask | toMask;
            board.occupancies[both] &= ~((toMask << 8) | (toMask >> 8));
            break;
        default: // promotions
            board.halfmoves = 0;
            if (flags & 0b100) { // capture
                board.pieces[board.board[to]] &= ~toMask; // update captured bitboard
                hash ^= pieceSqKey[board.board[to]][to];
                board.undoStack[board.ply].capturedPiece = board.board[to]; 
                board.occupancies[board.sideToMove^1] ^= toMask; // update enemy occupancy bitboard
                board.castling &= castlingUpdate[to]; // update castling rights (rook can be captured)
            }
            board.pieces[board.board[from]] ^= fromMask; // update pawn bitboard
            hash ^= pieceSqKey[board.board[from]][from];
            // update occupancy bitboards
            board.occupancies[board.sideToMove] ^= fromMask | toMask;
            board.occupancies[both] ^= fromMask;
            board.occupancies[both] |= toMask; // this bit can be set as we could have captured a piece 
            // place new piece
            board.board[to] = board.sideToMove *6 + (flags & 0b11); // flags contain info about promotion piece
            board.pieces[board.sideToMove *6 + (flags & 0b11)] ^= toMask;
            hash ^= pieceSqKey[board.board[to]][to];
            break;
    }

    hash ^= castlingKey[board.castling];

    if (board.sideToMove == black) board.fullmoves++;
    board.ply++;

    board.sideToMove ^= 1; // flip side to move
    hash ^= sideToMoveKey;
    board.hashStack[board.ply] = hash;
}


void unmakeMove(const uint16_t move, Board& board) {
    int from = move & 0b111111;
    int to = (move >> 6) & 0b111111;
    int flags = (move >> 12) & 0b1111;

    uint64_t fromMask = 1ULL << from;
    uint64_t toMask = 1ULL << to;

    Undo undo = board.undoStack[--board.ply];

    board.sideToMove ^= 1; // flip side to move

    if (board.sideToMove == black) board.fullmoves--;

    board.enPassant = undo.enPassant;
    board.halfmoves = undo.halfmoves;
    board.castling = undo.castling;

    switch (flags) {
        case 0: // quiet
        case 1: // double pawn push
            board.pieces[board.board[to]] ^= fromMask | toMask; // update bitboard
            // update occupancy bitboards
            board.occupancies[board.sideToMove] ^= fromMask | toMask;
            board.occupancies[both] ^= fromMask | toMask;
            // update board
            board.board[from] = board.board[to];
            break;
        case 2: // king castle
            // update bitboards
            board.pieces[board.board[to]] ^= fromMask | toMask; // king
            board.pieces[board.board[from+1]] ^= (toMask >> 1) | (toMask << 1); // rook
            // update occupancy bitboards (king + rook)
            board.occupancies[board.sideToMove] ^= fromMask | toMask | (toMask >> 1) | (toMask << 1);
            board.occupancies[both] ^= fromMask | toMask | (toMask >> 1) | (toMask << 1);
            // update board
            board.board[from] = board.board[to]; // king
            board.board[from+3] = board.board[from+1]; // rook
            break;
        case 3: // queen castle
            // update bitboards
            board.pieces[board.board[to]] ^= fromMask | toMask; // king
            board.pieces[board.board[from-1]] ^= (toMask >> 2) | (toMask << 1); // rook
            // update occupancy bitboards (king + rook)
            board.occupancies[board.sideToMove] ^= fromMask | toMask | (toMask >> 2) | (toMask << 1);
            board.occupancies[both] ^= fromMask | toMask | (toMask >> 2) | (toMask << 1);
            // update board
            board.board[from] = board.board[to]; // king
            board.board[from-4] = board.board[from-1]; // rook
            break;
        case 4: // captures
            board.pieces[board.board[to]] ^= fromMask | toMask; // update attacker bitboard
            board.pieces[undo.capturedPiece] ^= toMask; // update captured bitboard
            // update occupancy bitboards
            board.occupancies[board.sideToMove] ^= fromMask | toMask; // us
            board.occupancies[board.sideToMove^1] ^= toMask; // enemy
            board.occupancies[both] ^= fromMask;
            // update board
            board.board[from] = board.board[to];
            board.board[to] = undo.capturedPiece;
            break;
        case 5: // ep-capture
            board.pieces[board.board[to]] ^= fromMask | toMask; // update pawn bitboard
            board.board[from] = board.board[to]; // update board
            // update occupancy bitboards
            board.occupancies[board.sideToMove] ^= fromMask | toMask;
            board.occupancies[both] ^= fromMask | toMask;
            // restore captured pawn
            if (board.sideToMove == white) {
                board.pieces[p] ^= toMask >> 8;
                board.board[to-8] = p;
                board.occupancies[board.sideToMove^1] ^= toMask >> 8;
                board.occupancies[both] ^= toMask >> 8;
            }
            else {
                board.pieces[P] ^= toMask << 8;
                board.board[to+8] = P;
                board.occupancies[board.sideToMove^1] ^= toMask << 8;
                board.occupancies[both] ^= toMask << 8;
            }
            break;
        default: // promotions
            // remove new piece
            board.pieces[board.sideToMove*6 + (flags & 0b11)] ^= toMask;
            board.occupancies[board.sideToMove] ^= toMask;
            board.occupancies[both] ^= toMask;
            // restore pawn
            board.occupancies[board.sideToMove] ^= fromMask;
            board.occupancies[both] ^= fromMask;
            if (board.sideToMove == white) {
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
                board.occupancies[board.sideToMove^1] ^= toMask;
                board.occupancies[both] ^= toMask;
            }
            break;
    }
}