#include "board.h"
#include "constants.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <immintrin.h>


void loadFEN(const std::string& s, Board& board) {
    std::stringstream fen(s);

    std::fill(std::begin(board.pieces), std::end(board.pieces), 0ULL);

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
                case 'K': board.pieces[K] |= 1ULL << sq; board.board[sq] = K; break;
                case 'Q': board.pieces[Q] |= 1ULL << sq; board.board[sq] = Q; break;
                case 'B': board.pieces[B] |= 1ULL << sq; board.board[sq] = B; break;
                case 'N': board.pieces[N] |= 1ULL << sq; board.board[sq] = N; break;
                case 'R': board.pieces[R] |= 1ULL << sq; board.board[sq] = R; break;
                case 'P': board.pieces[P] |= 1ULL << sq; board.board[sq] = P; break;

                case 'k': board.pieces[k] |= 1ULL << sq; board.board[sq] = k; break;
                case 'q': board.pieces[q] |= 1ULL << sq; board.board[sq] = q; break;
                case 'b': board.pieces[b] |= 1ULL << sq; board.board[sq] = b; break;
                case 'n': board.pieces[n] |= 1ULL << sq; board.board[sq] = n; break;
                case 'r': board.pieces[r] |= 1ULL << sq; board.board[sq] = r; break;
                case 'p': board.pieces[p] |= 1ULL << sq; board.board[sq] = p; break;
            }
        }
    }

    getExtras(board);

    std::string movingSide; fen >> movingSide;
    board.sideToMove = (movingSide == "b"); // "b" = 1; "w" = 0

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

    std::string enPassant; fen >> enPassant;
    board.enPassantSquare = (enPassant == "-") ? -1 : (enPassant[1] -'1') * 8 + (enPassant[0] - 'a');

    std::string movesCnt;
    fen >> movesCnt; board.halfmoves = stoi(movesCnt);
    fen >> movesCnt; board.fullmoves = stoi(movesCnt);
}


void makeMove(const uint16_t move, Board& board) {
    int from = move & 0b111111;
    int to = (move >> 6) & 0b111111;
    int flags = (move >> 12) & 0b1111;

    uint64_t fromMask = 1ULL << from;
    uint64_t toMask = 1ULL << to;

    board.enPassantSquare = -1;

    switch (flags) {
        case 0: // quiet
            board.pieces[board.board[from]] ^= fromMask | toMask; // update bitboard
            board.board[to] = board.board[from]; // update board
            // update halfmove clock
            if (board.board[to] == p || board.board[to] == P) board.halfmoves = 0;
            else board.halfmoves++;
            // update castling rights
            board.castling &= castlingUpdate[from];
            break;
        case 1: // double pawn push
            board.halfmoves = 0;
            board.pieces[board.board[from]] ^= fromMask | toMask; // update bitboard
            board.board[to] = board.board[from]; // update board
            board.enPassantSquare = (from + to) / 2; // store ep square
            break;
        case 2: // king castle
            board.halfmoves++;
            // update bitboards
            board.pieces[board.board[from]] = toMask; // king
            board.pieces[board.board[from+3]] ^= (toMask >> 1) | (toMask << 1); // rook
            // update board
            board.board[to] = board.board[from]; // king
            board.board[from+1] = board.board[from+3]; // rook
            // update castling rights
            board.castling &= castlingUpdate[from];
            break;
        case 3: // queen castle
            board.halfmoves++;
            // update bitboards
            board.pieces[board.board[from]] = toMask; // king
            board.pieces[board.board[from-4]] ^= (toMask >> 2) | (toMask << 1); // rook
            // update board
            board.board[to] = board.board[from]; // king
            board.board[from-1] = board.board[from-4]; // rook
            // update castling rights
            board.castling &= castlingUpdate[from];
            break;
        case 4: // captures
            board.halfmoves = 0;
            board.pieces[board.board[from]] ^= fromMask | toMask; // update attacker bitboard
            board.pieces[board.board[to]] ^= toMask; // update captured bitboard
            board.board[to] = board.board[from]; // update board
            // update castling rights
            board.castling &= castlingUpdate[from]; // rook or king can capture
            board.castling &= castlingUpdate[to]; // rook can be captured
            break;
        case 5: // ep-capture
            board.halfmoves = 0;
            board.pieces[board.board[from]] ^= fromMask | toMask; // update pawn bitboard
            board.board[to] = board.board[from]; // update board
            // the if statement is unnecessary as to+8 and to-8 squares contain precisely one pawn - the captured one
            board.pieces[p] &= ~((toMask << 8) | (toMask >> 8));
            board.pieces[P] &= ~((toMask << 8) | (toMask >> 8));
            break;
        default: // promotions
            board.halfmoves = 0;
            if (flags & 0b100) { // capture
                board.pieces[board.board[to]] &= ~toMask; // update captured bitboard
                board.castling &= castlingUpdate[to]; // update castling rights (rook can be captured)
            }
            board.pieces[board.board[from]] ^= fromMask; // update pawn bitboard
            // place new piece
            board.board[to] = board.sideToMove *6 + (flags & 0b11); // flags contain info about promotion piece
            board.pieces[board.sideToMove *6 + (flags & 0b11)] ^= toMask;
            break;
    }

    if (board.sideToMove == black) board.fullmoves++;

    board.sideToMove ^= 1; // flip side to move

    getExtras(board);
}

bool isKingSafe(const Board& board, const int sq) {
    int us = board.sideToMove;
    int them = us ^ 1;

    bitboard danger = 0ULL;

    bitboard mask, pieces;

    // king
    danger |= kingLookup[sq] & board.pieces[us*6 + K];

    // knight
    danger |= knightLookup[sq] & board.pieces[us*6 + N];

    // rook + queen
    mask = rookLookup[sq];
    pieces = _pext_u64(board.occupancies[both], mask);
    danger |= rookMagic[sq][pieces] & (board.pieces[us*6 + R] | board.pieces[us*6 + Q]);

    // bishop + queen
    mask = bishopLookup[sq];
    pieces = _pext_u64(board.occupancies[both], mask);
    danger |= bishopMagic[sq][pieces] & (board.pieces[us*6 + B] | board.pieces[us*6 + Q]);

    mask = 1ULL << sq;

    // pawns
    if (board.sideToMove == white) {
        // left capture
        danger |= ((board.pieces[P] & ~FileA) << 7) & mask;
        // right capture
        danger |= ((board.pieces[P] & ~FileH) << 9) & mask;
    }
    else {
        // left capture
        danger |= ((board.pieces[p] & ~FileH) >> 7) & mask;
        // right capture
        danger |= ((board.pieces[p] & ~FileA) >> 9) & mask;
    }

    return (danger == 0);
}