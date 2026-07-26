#include "board.h"
#include "constants.h"
#include "utility.h"

#include <sstream>
#include <immintrin.h>


void getExtras(Board& board) {
    board.colour[0] = board.pieces[6] | board.pieces[7] | board.pieces[8] | board.pieces[9] | board.pieces[10] | board.pieces[11];
    board.colour[1] = board.pieces[0] | board.pieces[1] | board.pieces[2] | board.pieces[3] | board.pieces[4] | board.pieces[5];
    board.empty = ~(board.colour[0] | board.colour[1]);
}

void loadFEN(const std::string& s, Board& board) {
    std::stringstream fen(s);

    board.pieces[6] = board.pieces[7] = board.pieces[8] = board.pieces[9] = board.pieces[10] = board.pieces[11] = 0ULL;
    board.pieces[0] = board.pieces[1] = board.pieces[2] = board.pieces[3] = board.pieces[4] = board.pieces[5] = 0ULL;

    std::fill(std::begin(board.board), std::end(board.board), ' ');

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
            board.board[sq] = c;
            file++;
            switch(c) {
                case 'K': board.pieces[6] |= 1ULL << sq; break;
                case 'Q': board.pieces[7] |= 1ULL << sq; break;
                case 'B': board.pieces[8] |= 1ULL << sq; break;
                case 'N': board.pieces[9] |= 1ULL << sq; break;
                case 'R': board.pieces[10] |= 1ULL << sq; break;
                case 'P': board.pieces[11] |= 1ULL << sq; break;

                case 'k': board.pieces[0] |= 1ULL << sq; break;
                case 'q': board.pieces[1] |= 1ULL << sq; break;
                case 'b': board.pieces[2] |= 1ULL << sq; break;
                case 'n': board.pieces[3] |= 1ULL << sq; break;
                case 'r': board.pieces[4] |= 1ULL << sq; break;
                case 'p': board.pieces[5] |= 1ULL << sq; break;
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

    board.halfmoves++;
    if (!board.whiteToMove) board.fullmoves++;

    if (flags == 1) { // double pawn push;
        board.halfmoves = 0;
        if (board.whiteToMove) {
            board.pieces[11] &= ~fromMask; board.pieces[11] |= toMask;
            board.board[from] = ' '; board.board[to] = 'P';
            board.enPassantSquare = to - 8;
        }
        else {
            board.pieces[5] &= ~fromMask; board.pieces[5] |= toMask;
            board.board[from] = ' '; board.board[to] = 'p';
            board.enPassantSquare = to + 8;
        }
        board.whiteToMove = !board.whiteToMove;
        getExtras(board);
        return;
    }
    board.enPassantSquare = -1;

    if (flags == 2) { // king castle
        if (board.whiteToMove) {
            board.board[4] = board.board[7] = ' ';
            board.board[6] = 'K'; board.board[5] = 'R';
            board.pieces[6] = 1ULL << 6; board.pieces[10] &= ~(1ULL << 7); board.pieces[10] |= 1ULL << 5;
            board.cK = board.cQ = false;
        }
        else {
            board.board[60] = board.board[63] = ' ';
            board.board[62] = 'k'; board.board[61] = 'r';
            board.pieces[0] = 1ULL << 62; board.pieces[4] &= ~(1ULL << 63); board.pieces[4] |= 1ULL << 61;
            board.ck = board.cq = false;
        }
        board.whiteToMove = !board.whiteToMove;
        getExtras(board);
        return;
    }

    if (flags == 3) { // queen castle
        if (board.whiteToMove) {
            board.board[4] = board.board[0] = ' ';
            board.board[2] = 'K'; board.board[3] = 'R';
            board.pieces[6] = 1ULL << 2; board.pieces[10] &= ~1ULL; board.pieces[10] |= 1ULL << 3;
            board.cK = board.cQ = false;
        }
        else {
            board.board[60] = board.board[56] = ' ';
            board.board[58] = 'k'; board.board[59] = 'r';
            board.pieces[0] = 1ULL << 58; board.pieces[4] &= ~(1ULL << 56); board.pieces[4] |= 1ULL << 59;
            board.ck = board.cq = false;
        }
        board.whiteToMove = !board.whiteToMove;
        getExtras(board);
        return;
    }

    if (flags == 5) { // ep-capture
        board.halfmoves = 0;
        if (board.whiteToMove) {
            board.pieces[11] &= ~fromMask; board.pieces[11] |= toMask;
            board.board[from] = ' '; board.board[to] = 'P';
            board.pieces[5] &= ~(1ULL << (to - 8)); board.board[to - 8] = ' ';
        }
        else {
            board.pieces[5] &= ~fromMask; board.pieces[5] |= toMask;
            board.board[from] = ' '; board.board[to] = 'p';
            board.pieces[11] &= ~(1ULL << (to + 8)); board.board[to + 8] = ' ';
        }
        board.whiteToMove = !board.whiteToMove;
        getExtras(board);
        return;
    }

    if (flags & 0b100) { // capture
        board.halfmoves = 0;
        board.pieces[7] &= ~toMask; board.pieces[8] &= ~toMask; board.pieces[9] &= ~toMask; board.pieces[10] &= ~toMask; board.pieces[11] &= ~toMask;
        board.pieces[1] &= ~toMask; board.pieces[2] &= ~toMask; board.pieces[3] &= ~toMask; board.pieces[4] &= ~toMask; board.pieces[5] &= ~toMask;
        board.cK &= (board.pieces[10] & (1ULL << 7)) >> 7; board.cQ &= board.pieces[10] & 1ULL;
        board.ck &= (board.pieces[4] & (1ULL << 63)) >> 63; board.cq &= (board.pieces[4] & (1ULL << 56)) >> 56;
    }

    if (flags & 0b1000) { // promotion
        board.halfmoves = 0;
        if (board.whiteToMove) {
            board.pieces[11] &= ~fromMask;
            switch (flags & 0b11) {
                case 0: board.pieces[9] |= toMask; board.board[to] = 'N'; break;
                case 1: board.pieces[8] |= toMask; board.board[to] = 'B'; break;
                case 2: board.pieces[10] |= toMask; board.board[to] = 'R'; break;
                case 3: board.pieces[7] |= toMask; board.board[to] = 'Q'; break;
            }
        }
        else {
            board.pieces[5] &= ~fromMask;
            switch (flags & 0b11) {
                case 0: board.pieces[3] |= toMask; board.board[to] = 'n'; break;
                case 1: board.pieces[2] |= toMask; board.board[to] = 'b'; break;
                case 2: board.pieces[4] |= toMask; board.board[to] = 'r'; break;
                case 3: board.pieces[1] |= toMask; board.board[to] = 'q'; break;
            }
        }
        board.board[from] = ' ';
        board.whiteToMove = !board.whiteToMove;
        getExtras(board);
        return;
    }

    char piece = board.board[from];
    board.board[from] = ' '; board.board[to] = piece;
    switch (piece) {
        case 'K': board.pieces[6] &= ~fromMask; board.pieces[6] |= toMask; board.cK = board.cQ = false; break;
        case 'Q': board.pieces[7] &= ~fromMask; board.pieces[7] |= toMask; break;
        case 'B': board.pieces[8] &= ~fromMask; board.pieces[8] |= toMask; break;
        case 'N': board.pieces[9] &= ~fromMask; board.pieces[9] |= toMask; break;
        case 'R': board.pieces[10] &= ~fromMask; board.pieces[10] |= toMask; break;
        case 'P': board.pieces[11] &= ~fromMask; board.pieces[11] |= toMask; board.halfmoves = 0; break;

        case 'k': board.pieces[0] &= ~fromMask; board.pieces[0] |= toMask; board.ck = board.cq = false; break;
        case 'q': board.pieces[1] &= ~fromMask; board.pieces[1] |= toMask; break;
        case 'b': board.pieces[2] &= ~fromMask; board.pieces[2] |= toMask; break;
        case 'n': board.pieces[3] &= ~fromMask; board.pieces[3] |= toMask; break;
        case 'r': board.pieces[4] &= ~fromMask; board.pieces[4] |= toMask; break;
        case 'p': board.pieces[5] &= ~fromMask; board.pieces[5] |= toMask; board.halfmoves = 0; break;
    }
    board.cK &= (board.pieces[10] & (1ULL << 7)) >> 7; board.cQ &= board.pieces[10] & 1ULL;
    board.ck &= (board.pieces[4] & (1ULL << 63)) >> 63; board.cq &= (board.pieces[4] & (1ULL << 56)) >> 56;

    board.whiteToMove = !board.whiteToMove;
    getExtras(board);
}

bool isKingSafe(Board& board) {
    int us = board.whiteToMove;
    int them = us ^ 1;

    int enemyKingSq = __builtin_ctzll(board.pieces[them*6]);

    bitboard danger = 0ULL;

    bitboard mask, pieces;

    // king
    danger |= kingLookup[enemyKingSq] & board.pieces[us*6];

    // knight
    danger |= knightLookup[enemyKingSq] & board.pieces[us*6 + 3];

    // rook + queen
    mask = rookLookup[enemyKingSq];
    pieces = _pext_u64(~board.empty, mask);
    danger |= rookMagic[enemyKingSq][pieces] & (board.pieces[us*6 + 4] | board.pieces[us*6 + 1]);

    // bishop + queen
    mask = bishopLookup[enemyKingSq];
    pieces = _pext_u64(~board.empty, mask);
    danger |= bishopMagic[enemyKingSq][pieces] & (board.pieces[us*6 + 2] | board.pieces[us*6 + 1]);

    // pawns
    if (board.whiteToMove) {
        // left capture
        danger |= ((board.pieces[11] & ~FileA) << 7) & board.pieces[them*6];
        // right capture
        danger |= ((board.pieces[11] & ~FileH) << 9) & board.pieces[them*6];
    }
    else {
        // left capture
        danger |= ((board.pieces[5] & ~FileH) >> 7) & board.pieces[them*6];
        // right capture
        danger |= ((board.pieces[5] & ~FileA) >> 9) & board.pieces[them*6];
    }

    return (danger == 0);
}

bitboard getDangerSquares(Board& board) {
    uint64_t danger = 0ULL;
    uint64_t moveMask = 0ULL;
    int sq;
    bitboard mask;
    uint64_t pieces;
    int us = board.whiteToMove;
    int them = us ^ 1;

    // king
    danger |= kingLookup[__builtin_ctzll(board.pieces[them*6])];

    // knight
    moveMask = board.pieces[them*6 + 3];
    while (moveMask) {
        sq = popLSB(moveMask);
        danger |= knightLookup[sq];
    }

    // rook + queen
    moveMask = board.pieces[them*6 + 4] | board.pieces[them*6 + 1];
    while (moveMask) {
        sq = popLSB(moveMask);
        mask = rookLookup[sq];
        pieces = _pext_u64(~board.empty, mask);
        danger |= rookMagic[sq][pieces];
    }

    // bishop + queen
    moveMask = board.pieces[them*6 + 2] | board.pieces[them*6 + 1];
    while (moveMask) {
        sq = popLSB(moveMask);
        mask = bishopLookup[sq];
        pieces = _pext_u64(~board.empty, mask);
        danger |= bishopMagic[sq][pieces];
    }

    if (board.whiteToMove) {
        // pawns
        // left capture
        danger |= (board.pieces[5] & ~FileH) >> 7;
        // right capture
        danger |= (board.pieces[5] & ~FileA) >> 9;
    }
    else {
        // pawns
        // left capture
        danger |= (board.pieces[11] & ~FileA) << 7;
        // right capture
        danger |= (board.pieces[11] & ~FileH) << 9;
    }

    return danger;
}