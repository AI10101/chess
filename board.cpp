#include "board.h"
#include "constants.h"

#include <sstream>
#include <immintrin.h>


void loadFEN(const std::string& s, Board& board) {
    std::stringstream fen(s);

    board.pieces[6] = board.pieces[7] = board.pieces[8] = board.pieces[9] = board.pieces[10] = board.pieces[11] = 0ULL;
    board.pieces[0] = board.pieces[1] = board.pieces[2] = board.pieces[3] = board.pieces[4] = board.pieces[5] = 0ULL;

    std::fill(std::begin(board.board), std::end(board.board), 12);

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
                case 'K': board.pieces[6] |= 1ULL << sq; board.board[sq] = 6; break;
                case 'Q': board.pieces[7] |= 1ULL << sq; board.board[sq] = 7; break;
                case 'B': board.pieces[8] |= 1ULL << sq; board.board[sq] = 8; break;
                case 'N': board.pieces[9] |= 1ULL << sq; board.board[sq] = 9; break;
                case 'R': board.pieces[10] |= 1ULL << sq; board.board[sq] = 10; break;
                case 'P': board.pieces[11] |= 1ULL << sq; board.board[sq] = 11; break;

                case 'k': board.pieces[0] |= 1ULL << sq; board.board[sq] = 0; break;
                case 'q': board.pieces[1] |= 1ULL << sq; board.board[sq] = 1; break;
                case 'b': board.pieces[2] |= 1ULL << sq; board.board[sq] = 2; break;
                case 'n': board.pieces[3] |= 1ULL << sq; board.board[sq] = 3; break;
                case 'r': board.pieces[4] |= 1ULL << sq; board.board[sq] = 4; break;
                case 'p': board.pieces[5] |= 1ULL << sq; board.board[sq] = 5; break;
            }
        }
    }

    std::string sideToMove; fen >> sideToMove;
    board.whiteToMove = (sideToMove == "w");

    std::string castling; fen >> castling;
    board.cK = board.cQ = board.ck = board.cq = false;
    for (char c: castling) {
        switch (c) {
            case 'K': board.cK = true; break;
            case 'Q': board.cQ = true; break;
            case 'k': board.ck = true; break;
            case 'q': board.cq = true; break;
        }
    }

    std::string enPassant; fen >> enPassant;
    board.enPassantSquare = (enPassant == "-") ? -1 : (enPassant[1] -'1') * 8 + (enPassant[0] - 'a');

    std::string moves;
    fen >> moves; board.halfmoves = stoi(moves);
    fen >> moves; board.fullmoves = stoi(moves);

    getExtras(board);
}

void makeMove(const uint16_t& move, Board& board) {
    int from = move & 0b111111;
    int to = (move >> 6) & 0b111111;
    int flags = (move >> 12) & 0b1111;

    uint64_t fromMask = 1ULL << from;
    uint64_t toMask = 1ULL << to;

    int us = board.whiteToMove;

    board.halfmoves++;
    if (!board.whiteToMove) board.fullmoves++;

    board.enPassantSquare = -1;

    switch (flags) {
        case 0:
            board.pieces[board.board[from]] ^= fromMask | toMask;
            board.board[to] = board.board[from]; board.board[from] = 12;
            if (board.board[to] == 0) board.ck = board.cq = false;
            if (board.board[to] == 6) board.cK = board.cQ = false;
            if (board.board[to] == 5 || board.board[to] == 11) board.halfmoves = 0;
            board.cK &= (board.pieces[10] & (1ULL << 7)) >> 7; board.cQ &= board.pieces[10] & 1ULL;
            board.ck &= (board.pieces[4] & (1ULL << 63)) >> 63; board.cq &= (board.pieces[4] & (1ULL << 56)) >> 56;
            break;
        case 1:
            board.halfmoves = 0;
            board.pieces[6*us + 5] ^= fromMask | toMask;
            board.board[to] = board.board[from]; board.board[from] = 12;
            board.enPassantSquare = (from + to) / 2;
            break;
        case 2:
            if (board.whiteToMove) {
                board.board[4] = board.board[7] = 12;
                board.board[6] = 6; board.board[5] = 10;
                board.pieces[6] = 1ULL << 6; board.pieces[10] &= ~(1ULL << 7); board.pieces[10] |= 1ULL << 5;
                board.cK = board.cQ = false;
            }
            else {
                board.board[60] = board.board[63] = 12;
                board.board[62] = 0; board.board[61] = 4;
                board.pieces[0] = 1ULL << 62; board.pieces[4] &= ~(1ULL << 63); board.pieces[4] |= 1ULL << 61;
                board.ck = board.cq = false;
            }
            break;
        case 3:
            if (board.whiteToMove) {
                board.board[4] = board.board[0] = 12;
                board.board[2] = 6; board.board[3] = 10;
                board.pieces[6] = 1ULL << 2; board.pieces[10] &= ~1ULL; board.pieces[10] |= 1ULL << 3;
                board.cK = board.cQ = false;
            }
            else {
                board.board[60] = board.board[56] = 12;
                board.board[58] = 0; board.board[59] = 4;
                board.pieces[0] = 1ULL << 58; board.pieces[4] &= ~(1ULL << 56); board.pieces[4] |= 1ULL << 59;
                board.ck = board.cq = false;
            }
            break;
        case 4:
            board.pieces[board.board[from]] ^= fromMask | toMask;
            board.pieces[board.board[to]] ^= toMask;
            board.board[to] = board.board[from]; board.board[from] = 12;
            if (board.board[to] == 0) board.ck = board.cq = false;
            if (board.board[to] == 6) board.cK = board.cQ = false;
            if (board.board[to] == 5 || board.board[to] == 11) board.halfmoves = 0;
            board.cK &= (board.pieces[10] & (1ULL << 7)) >> 7; board.cQ &= board.pieces[10] & 1ULL;
            board.ck &= (board.pieces[4] & (1ULL << 63)) >> 63; board.cq &= (board.pieces[4] & (1ULL << 56)) >> 56;
            break;
        case 5:
            board.halfmoves = 0;
            if (board.whiteToMove) {
                board.pieces[11] &= ~fromMask; board.pieces[11] |= toMask;
                board.board[from] = 12; board.board[to] = 11;
                board.pieces[5] &= ~(1ULL << (to - 8)); board.board[to - 8] = 12;
            }
            else {
                board.pieces[5] &= ~fromMask; board.pieces[5] |= toMask;
                board.board[from] = 12; board.board[to] = 5;
                board.pieces[11] &= ~(1ULL << (to + 8)); board.board[to + 8] = 12;
            }
            break;
        default:
            board.halfmoves = 0;
            if (flags & 0b100) {
                board.pieces[board.board[to]] ^= toMask;
                board.cK &= (board.pieces[10] & (1ULL << 7)) >> 7; board.cQ &= board.pieces[10] & 1ULL;
                board.ck &= (board.pieces[4] & (1ULL << 63)) >> 63; board.cq &= (board.pieces[4] & (1ULL << 56)) >> 56;
            }
            if (board.whiteToMove) {
                board.pieces[11] &= ~fromMask;
                switch (flags & 0b11) {
                    case 0: board.pieces[9] |= toMask; board.board[to] = 9; break;
                    case 1: board.pieces[8] |= toMask; board.board[to] = 8; break;
                    case 2: board.pieces[10] |= toMask; board.board[to] = 10; break;
                    case 3: board.pieces[7] |= toMask; board.board[to] = 7; break;
                }
            }
            else {
                board.pieces[5] &= ~fromMask;
                switch (flags & 0b11) {
                    case 0: board.pieces[3] |= toMask; board.board[to] = 3; break;
                    case 1: board.pieces[2] |= toMask; board.board[to] = 2; break;
                    case 2: board.pieces[4] |= toMask; board.board[to] = 4; break;
                    case 3: board.pieces[1] |= toMask; board.board[to] = 1; break;
                }
            }
            board.board[from] = 12;
            break;
    }

    board.whiteToMove = !board.whiteToMove;
    getExtras(board);
}

bool isKingSafe(const Board& board, const int sq) {
    int us = board.whiteToMove;
    int them = us ^ 1;

    bitboard danger = 0ULL;

    bitboard mask, pieces;

    // king
    danger |= kingLookup[sq] & board.pieces[us*6];

    // knight
    danger |= knightLookup[sq] & board.pieces[us*6 + 3];

    // rook + queen
    mask = rookLookup[sq];
    pieces = _pext_u64(~board.empty, mask);
    danger |= rookMagic[sq][pieces] & (board.pieces[us*6 + 4] | board.pieces[us*6 + 1]);

    // bishop + queen
    mask = bishopLookup[sq];
    pieces = _pext_u64(~board.empty, mask);
    danger |= bishopMagic[sq][pieces] & (board.pieces[us*6 + 2] | board.pieces[us*6 + 1]);

    mask = 1ULL << sq;

    // pawns
    if (board.whiteToMove) {
        // left capture
        danger |= ((board.pieces[11] & ~FileA) << 7) & mask;
        // right capture
        danger |= ((board.pieces[11] & ~FileH) << 9) & mask;
    }
    else {
        // left capture
        danger |= ((board.pieces[5] & ~FileH) >> 7) & mask;
        // right capture
        danger |= ((board.pieces[5] & ~FileA) >> 9) & mask;
    }

    return (danger == 0);
}