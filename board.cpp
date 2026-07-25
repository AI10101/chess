#include "board.h"
#include "constants.h"
#include "utility.h"

#include <sstream>
#include <immintrin.h>


void getExtras(Board& board) {
    board.white = board.K | board.Q | board.B | board.N | board.R | board.P;
    board.black = board.k | board.q | board.b | board.n | board.r | board.p;
    board.empty = ~(board.white | board.black);
}

void loadFEN(const std::string& s, Board& board) {
    std::stringstream fen(s);

    board.K = board.Q = board.B = board.N = board.R = board.P = 0ULL;
    board.k = board.q = board.b = board.n = board.r = board.p = 0ULL;

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
                case 'K': board.K |= 1ULL << sq; break;
                case 'Q': board.Q |= 1ULL << sq; break;
                case 'B': board.B |= 1ULL << sq; break;
                case 'N': board.N |= 1ULL << sq; break;
                case 'R': board.R |= 1ULL << sq; break;
                case 'P': board.P |= 1ULL << sq; break;

                case 'k': board.k |= 1ULL << sq; break;
                case 'q': board.q |= 1ULL << sq; break;
                case 'b': board.b |= 1ULL << sq; break;
                case 'n': board.n |= 1ULL << sq; break;
                case 'r': board.r |= 1ULL << sq; break;
                case 'p': board.p |= 1ULL << sq; break;
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
            board.P &= ~fromMask; board.P |= toMask;
            board.board[from] = ' '; board.board[to] = 'P';
            board.enPassantSquare = to - 8;
        }
        else {
            board.p &= ~fromMask; board.p |= toMask;
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
            board.K = 1ULL << 6; board.R &= ~(1ULL << 7); board.R |= 1ULL << 5;
            board.cK = board.cQ = false;
        }
        else {
            board.board[60] = board.board[63] = ' ';
            board.board[62] = 'k'; board.board[61] = 'r';
            board.k = 1ULL << 62; board.r &= ~(1ULL << 63); board.r |= 1ULL << 61;
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
            board.K = 1ULL << 2; board.R &= ~1ULL; board.R |= 1ULL << 3;
            board.cK = board.cQ = false;
        }
        else {
            board.board[60] = board.board[56] = ' ';
            board.board[58] = 'k'; board.board[59] = 'r';
            board.k = 1ULL << 58; board.r &= ~(1ULL << 56); board.r |= 1ULL << 59;
            board.ck = board.cq = false;
        }
        board.whiteToMove = !board.whiteToMove;
        getExtras(board);
        return;
    }

    if (flags == 5) { // ep-capture
        board.halfmoves = 0;
        if (board.whiteToMove) {
            board.P &= ~fromMask; board.P |= toMask;
            board.board[from] = ' '; board.board[to] = 'P';
            board.p &= ~(1ULL << (to - 8)); board.board[to - 8] = ' ';
        }
        else {
            board.p &= ~fromMask; board.p |= toMask;
            board.board[from] = ' '; board.board[to] = 'p';
            board.P &= ~(1ULL << (to + 8)); board.board[to + 8] = ' ';
        }
        board.whiteToMove = !board.whiteToMove;
        getExtras(board);
        return;
    }

    if (flags & 0b100) { // capture
        board.halfmoves = 0;
        board.Q &= ~toMask; board.B &= ~toMask; board.N &= ~toMask; board.R &= ~toMask; board.P &= ~toMask;
        board.q &= ~toMask; board.b &= ~toMask; board.n &= ~toMask; board.r &= ~toMask; board.p &= ~toMask;
        board.cK &= (board.R & (1ULL << 7)) >> 7; board.cQ &= board.R & 1ULL;
        board.ck &= (board.r & (1ULL << 63)) >> 63; board.cq &= (board.r & (1ULL << 56)) >> 56;
    }

    if (flags & 0b1000) { // promotion
        board.halfmoves = 0;
        if (board.whiteToMove) {
            board.P &= ~fromMask;
            switch (flags & 0b11) {
                case 0: board.N |= toMask; board.board[to] = 'N'; break;
                case 1: board.B |= toMask; board.board[to] = 'B'; break;
                case 2: board.R |= toMask; board.board[to] = 'R'; break;
                case 3: board.Q |= toMask; board.board[to] = 'Q'; break;
            }
        }
        else {
            board.p &= ~fromMask;
            switch (flags & 0b11) {
                case 0: board.n |= toMask; board.board[to] = 'n'; break;
                case 1: board.b |= toMask; board.board[to] = 'b'; break;
                case 2: board.r |= toMask; board.board[to] = 'r'; break;
                case 3: board.q |= toMask; board.board[to] = 'q'; break;
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
        case 'K': board.K &= ~fromMask; board.K |= toMask; board.cK = board.cQ = false; break;
        case 'Q': board.Q &= ~fromMask; board.Q |= toMask; break;
        case 'B': board.B &= ~fromMask; board.B |= toMask; break;
        case 'N': board.N &= ~fromMask; board.N |= toMask; break;
        case 'R': board.R &= ~fromMask; board.R |= toMask; break;
        case 'P': board.P &= ~fromMask; board.P |= toMask; board.halfmoves = 0; break;

        case 'k': board.k &= ~fromMask; board.k |= toMask; board.ck = board.cq = false; break;
        case 'q': board.q &= ~fromMask; board.q |= toMask; break;
        case 'b': board.b &= ~fromMask; board.b |= toMask; break;
        case 'n': board.n &= ~fromMask; board.n |= toMask; break;
        case 'r': board.r &= ~fromMask; board.r |= toMask; break;
        case 'p': board.p &= ~fromMask; board.p |= toMask; board.halfmoves = 0; break;
    }
    board.cK &= (board.R & (1ULL << 7)) >> 7; board.cQ &= board.R & 1ULL;
    board.ck &= (board.r & (1ULL << 63)) >> 63; board.cq &= (board.r & (1ULL << 56)) >> 56;

    board.whiteToMove = !board.whiteToMove;
    getExtras(board);
}

bitboard getDangerSquares(Board& board) {
    uint64_t danger = 0ULL;
    uint64_t moveMask = 0ULL;
    int sq;
    bitboard mask;
    uint64_t pieces;

    if (board.whiteToMove) {
        // pawns
        // left capture
        danger |= (board.p & ~FileH) >> 7;
        // right capture
        danger |= (board.p & ~FileA) >> 9;

        // king
        danger |= kingLookup[__builtin_ctzll(board.k)];

        // knight
        moveMask = board.n;
        while (moveMask) {
            sq = popLSB(moveMask);
            danger |= knightLookup[sq];
        }

        // rook + queen
        moveMask = board.r | board.q;
        while (moveMask) {
            sq = popLSB(moveMask);
            mask = rookLookup[sq];
            pieces = _pext_u64(~board.empty, mask);
            danger |= rookMagic[sq][pieces];
        }

        // bishop + queen
        moveMask = board.b | board.q;
        while (moveMask) {
            sq = popLSB(moveMask);
            mask = bishopLookup[sq];
            pieces = _pext_u64(~board.empty, mask);
            danger |= bishopMagic[sq][pieces];
        }
    }
    else {
        // pawns
        // left capture
        danger |= (board.P & ~FileA) << 7;
        // right capture
        danger |= (board.P & ~FileH) << 9;

        // king
        danger |= kingLookup[__builtin_ctzll(board.K)];

        // knight
        moveMask = board.N;
        while (moveMask) {
            sq = popLSB(moveMask);
            danger |= knightLookup[sq];
        }

        // rook + queen
        moveMask = board.R | board.Q;
        while (moveMask) {
            sq = popLSB(moveMask);
            mask = rookLookup[sq];
            pieces = _pext_u64(~board.empty, mask);
            danger |= rookMagic[sq][pieces];
        }

        // bishop + queen
        moveMask = board.B | board.Q;
        while (moveMask) {
            sq = popLSB(moveMask);
            mask = bishopLookup[sq];
            pieces = _pext_u64(~board.empty, mask);
            danger |= bishopMagic[sq][pieces];
        }
    }

    return danger;
}