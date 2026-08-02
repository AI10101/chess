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

inline void quiet(Board& board, int from, int to, bitboard fromMask, bitboard toMask) {
    board.pieces[board.board[from]] ^= fromMask | toMask; // update bitboard
    board.board[to] = board.board[from]; board.board[from] = 12; // update board
    // update halfmove clock
    if (board.board[to] == 5 || board.board[to] == 11) board.halfmoves = 0;
    else board.halfmoves++;
    // update castling rights
    switch (board.board[to]) {
        case 0: board.ck = board.cq = false; break;
        case 6: board.cK = board.cQ = false; break;
        case 10: board.cK &= board.pieces[10] >> 7; board.cQ &= board.pieces[10]; break;
        case 4: board.ck &= board.pieces[4] >> 63; board.cq &= board.pieces[4] >> 56; break;
    }
}

inline void capture(Board& board, int from, int to, bitboard fromMask, bitboard toMask) {
    board.halfmoves = 0;
    board.pieces[board.board[from]] ^= fromMask | toMask; // update attacker bitboard
    board.pieces[board.board[to]] ^= toMask; // update captured bitboard
    board.board[to] = board.board[from]; board.board[from] = 12; // update board
    switch (board.board[to]) {
        // update castling rights
        case 0: board.ck = board.cq = false; break;
        case 6: board.cK = board.cQ = false; break;
    }
    board.cK &= board.pieces[10] >> 7; board.cQ &= board.pieces[10] & 1ULL;
    board.ck &= board.pieces[4] >> 63; board.cq &= board.pieces[4] >> 56;
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
            quiet(board, from, to, fromMask, toMask);
            break;
        case 4: // captures
            capture(board, from, to, fromMask, toMask);
            break;
        case 1: // double pawn push
            board.halfmoves = 0;
            board.pieces[board.board[from]] ^= fromMask | toMask; // update bitboard
            board.board[to] = board.board[from]; board.board[from] = 12; // update board
            board.enPassantSquare = (from + to) / 2; // store ep square
            break;
        case 2: // king castle
            board.halfmoves++;
            // update bitboards
            board.pieces[board.board[from]] = toMask;
            board.pieces[board.board[from+3]] ^= (toMask >> 1) | (toMask << 1);
            // update board
            board.board[to] = board.board[from];
            board.board[from+1] = board.board[from+3];
            board.board[from] = board.board[from+3] = 12;
            // update castling rights
            if (board.whiteToMove) board.cK = board.cQ = false;
            else board.ck = board.cq = false;
            break;
        case 3: // queen castle
            board.halfmoves++;
            // update bitboards
            board.pieces[board.board[from]] = toMask;
            board.pieces[board.board[from-4]] ^= (toMask >> 2) | (toMask << 1);
            // update board
            board.board[to] = board.board[from];
            board.board[from-1] = board.board[from-4];
            board.board[from] = board.board[from-4] = 12;
            // update castling rights
            if (board.whiteToMove) board.cK = board.cQ = false;
            else board.ck = board.cq = false;
            break;
        case 5: // ep-capture
            board.halfmoves = 0;
            board.pieces[board.board[from]] ^= fromMask | toMask; // update pawn bitboard
            board.board[to] = board.board[from]; board.board[from] = 12; // update board
            board.board[to - 8] = 12;
            if (board.whiteToMove) {
                board.pieces[5] &= ~(1ULL << (to - 8)); board.board[to - 8] = 12;
            }
            else {
                board.pieces[11] &= ~(1ULL << (to + 8)); board.board[to + 8] = 12;
            }
            break;
        default: // promotions
            board.halfmoves = 0;
            if (flags & 0b100) {
                board.pieces[board.board[to]] &= ~toMask;
                switch (board.board[to]) {
                    case 10: board.cK &= board.pieces[10] >> 7; board.cQ &= board.pieces[10]; break;
                    case 4: board.ck &= board.pieces[4] >> 63; board.cq &= board.pieces[4] >> 56; break;
                }
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

    if (!board.whiteToMove) board.fullmoves++;

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