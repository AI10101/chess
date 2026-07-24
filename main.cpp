#include <bits/stdc++.h>
#include <cstdint>
using namespace std;


uint64_t Rank1 = 0xff;
uint64_t Rank2 = 0xff00;
uint64_t Rank7 = 0xff000000000000;
uint64_t Rank8 = 0xff00000000000000;

uint64_t FileA = 0x101010101010101;
uint64_t FileB = 0x202020202020202;
uint64_t FileG = 0x4040404040404040;
uint64_t FileH = 0x8080808080808080;


uint64_t kingLookup[64];

void getKingLookup() {
    for (int sq=0; sq < 64; sq++) {
        uint64_t mask = 1ULL << sq;
        uint64_t attacks = 0ULL;

        attacks |= (mask & ~FileH) << 1; // right
        attacks |= (mask & ~FileA) >> 1; // left
        attacks |= (mask & ~Rank8) << 8; // up
        attacks |= (mask & ~Rank1) >> 8; // down
        attacks |= ((mask & ~FileH) & ~Rank8) << 9; // right-up
        attacks |= ((mask & ~FileA) & ~Rank8) << 7; // left-up
        attacks |= ((mask & ~FileH) & ~Rank1) >> 7; // right-down
        attacks |= ((mask & ~FileA) & ~Rank1) >> 9; // left-down

        kingLookup[sq] = attacks;
    }
}


class Board {
  public:
    char board[64];
    bool whiteToMove;
    bool cK, cQ, ck, cq; // castling
    int enPassantSquare;
    int halfmoves, fullmoves;

    // bitboards
    uint64_t K, Q, B, N, R, P; // white pieces
    uint64_t k, q, b, n, r, p; // black pieces
    uint64_t white, black, empty;


    Board() {
        loadFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    }

    void loadFEN(const string& s) {
        stringstream fen(s);

        K=Q=B=N=R=P = 0ULL;
        k=q=b=n=r=p = 0ULL;

        fill(begin(board), end(board), ' ');

        string placement; fen >> placement;
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
                board[sq] = c;
                file++;
                switch(c) {
                    case 'K': K |= 1ULL << sq; break;
                    case 'Q': Q |= 1ULL << sq; break;
                    case 'B': B |= 1ULL << sq; break;
                    case 'N': N |= 1ULL << sq; break;
                    case 'R': R |= 1ULL << sq; break;
                    case 'P': P |= 1ULL << sq; break;

                    case 'k': k |= 1ULL << sq; break;
                    case 'q': q |= 1ULL << sq; break;
                    case 'b': b |= 1ULL << sq; break;
                    case 'n': n |= 1ULL << sq; break;
                    case 'r': r |= 1ULL << sq; break;
                    case 'p': p |= 1ULL << sq; break;
                }
            }
        }

        string sideToMove; fen >> sideToMove;
        whiteToMove = (sideToMove == "w");

        string castling; fen >> castling;
        cK = cQ = ck = cq = false;
        for (char c: castling) {
            switch (c) {
                case 'K': cK = true; break;
                case 'Q': cQ = true; break;
                case 'k': ck = true; break;
                case 'q': cq = true; break;
            }
        }

        string enPassant; fen >> enPassant;
        enPassantSquare = (enPassant == "-") ? -1 : (enPassant[1] -'1') * 8 + (enPassant[0] - 'a');

        string moves;
        fen >> moves; halfmoves = stoi(moves);
        fen >> moves; fullmoves = stoi(moves);

        getExtras();
    }

    void getExtras() {
        white = K | Q | B | N | R | P;
        black = k | q | b | n | r | p;
        empty = ~(white | black);
    }

    void printBoard() {
        for (int i = 0; i < 8; i++) cout << "+---";
        cout << "+\n";

        for (int rank = 7; rank >= 0; rank--) {
            for (int file = 0; file < 8; file++) {
                cout << "| " << board[rank * 8 + file] << " ";
            }
            cout << "| " << rank + 1 << "\n";
            for (int i = 0; i < 8; i++) cout << "+---";
            cout << "+\n";
        }

        for (char c = 'a'; c <= 'h'; c++) {
            cout << "  " << c << " ";
        }
        cout << "\n";
    }

    void printBitboard(const uint64_t& bitboard) {
        for (int rank = 7; rank >= 0; rank--) {
            for (int file = 0; file < 8; file++) {
                int sq = rank * 8 + file;
                cout << ((bitboard >> sq) & 1ULL);
            }
            cout << "\n";
        }
    }

    void makeMove(const uint16_t& move) {
        int from = move & 0b111111;
        int to = (move >> 6) & 0b111111;
        int flags = (move >> 12) & 0b1111;

        uint64_t fromMask = 1ULL << from;
        uint64_t toMask = 1ULL << to;

        halfmoves++;
        if (!whiteToMove) fullmoves++;

        if (flags == 1) { // double pawn push;
            halfmoves = 0;
            if (whiteToMove) {
                P &= ~fromMask; P |= toMask;
                board[from] = ' '; board[to] = 'P';
                enPassantSquare = to - 8;
            }
            else {
                p &= ~fromMask; p |= toMask;
                board[from] = ' '; board[to] = 'p';
                enPassantSquare = to + 8;
            }
            whiteToMove = !whiteToMove;
            getExtras();
            return;
        }
        enPassantSquare = -1;

        if (flags == 2) { // king castle
            if (whiteToMove) {
                board[4] = board[7] = ' ';
                board[6] = 'K'; board[5] = 'R';
                K = 1ULL << 6; R &= ~(1ULL << 7); R |= 1ULL << 5;
                cK = cQ = false;
            }
            else {
                board[60] = board[63] = ' ';
                board[62] = 'k'; board[61] = 'r';
                k = 1ULL << 62; r &= ~(1ULL << 63); r |= 1ULL << 61;
                ck = cq = false;
            }
            whiteToMove = !whiteToMove;
            getExtras();
            return;
        }

        if (flags == 3) { // queen castle
            if (whiteToMove) {
                board[4] = board[0] = ' ';
                board[2] = 'K'; board[3] = 'R';
                K = 1ULL << 2; R &= ~1ULL; R |= 1ULL << 3;
                cK = cQ = false;
            }
            else {
                board[60] = board[56] = ' ';
                board[58] = 'k'; board[59] = 'r';
                k = 1ULL << 58; r &= ~(1ULL << 56); r |= 1ULL << 59;
                ck = cq = false;
            }
            whiteToMove = !whiteToMove;
            getExtras();
            return;
        }

        if (flags == 5) { // ep-capture
            halfmoves = 0;
            if (whiteToMove) {
                P &= ~fromMask; P |= toMask;
                board[from] = ' '; board[to] = 'P';
                p &= ~(1ULL << (to - 8)); board[to - 8] = ' ';
            }
            else {
                p &= ~fromMask; p |= toMask;
                board[from] = ' '; board[to] = 'p';
                P &= ~(1ULL << (to + 8)); board[to + 8] = ' ';
            }
            whiteToMove = !whiteToMove;
            getExtras();
            return;
        }

        if (flags & 0b100) { // capture
            halfmoves = 0;
            Q &= ~toMask; B &= ~toMask; N &= ~toMask; R &= ~toMask; P &= ~toMask;
            q &= ~toMask; b &= ~toMask; n &= ~toMask; r &= ~toMask; p &= ~toMask;
            cK &= (R & (1ULL << 7)) >> 7; cQ &= R & 1ULL;
            ck &= (r & (1ULL << 63)) >> 63; cq &= (r & (1ULL << 56)) >> 56;
        }

        if (flags & 0b1000) { // promotion
            halfmoves = 0;
            if (whiteToMove) {
                P &= ~fromMask;
                switch (flags & 0b11) {
                    case 0: N |= toMask; board[to] = 'N'; break;
                    case 1: B |= toMask; board[to] = 'B'; break;
                    case 2: R |= toMask; board[to] = 'R'; break;
                    case 3: Q |= toMask; board[to] = 'Q'; break;
                }
            }
            else {
                p &= ~fromMask;
                switch (flags & 0b11) {
                    case 0: n |= toMask; board[to] = 'n'; break;
                    case 1: b |= toMask; board[to] = 'b'; break;
                    case 2: r |= toMask; board[to] = 'r'; break;
                    case 3: q |= toMask; board[to] = 'q'; break;
                }
            }
            board[from] = ' ';
            whiteToMove = !whiteToMove;
            getExtras();
            return;
        }

        char piece = board[from];
        board[from] = ' '; board[to] = piece;
        switch (piece) {
            case 'K': K &= ~fromMask; K |= toMask; cK = cQ = false; break;
            case 'Q': Q &= ~fromMask; Q |= toMask; break;
            case 'B': B &= ~fromMask; B |= toMask; break;
            case 'N': N &= ~fromMask; N |= toMask; break;
            case 'R': R &= ~fromMask; R |= toMask; break;
            case 'P': P &= ~fromMask; P |= toMask; halfmoves = 0; break;

            case 'k': k &= ~fromMask; k |= toMask; ck = cq = false; break;
            case 'q': q &= ~fromMask; q |= toMask; break;
            case 'b': b &= ~fromMask; b |= toMask; break;
            case 'n': n &= ~fromMask; n |= toMask; break;
            case 'r': r &= ~fromMask; r |= toMask; break;
            case 'p': p &= ~fromMask; p |= toMask; halfmoves = 0; break;
        }
        cK &= (R & (1ULL << 7)) >> 7; cQ &= R & 1ULL;
        ck &= (r & (1ULL << 63)) >> 63; cq &= (r & (1ULL << 56)) >> 56;

        whiteToMove = !whiteToMove;
        getExtras();
    }

    int popLSB(uint64_t& bb) {
        int sq = __builtin_ctzll(bb);
        bb &= bb - 1;
        return sq;
    }

    void addMoves(vector<uint16_t>& moves, uint64_t moveMask, int offset, uint16_t flags = 0b0000) {
        while (moveMask) {
            int sq = popLSB(moveMask);
            moves.push_back((flags << 12) | (sq << 6) | (sq + offset));
        }
    }

    void addMovesFromAttackMask(vector<uint16_t>& moves, uint64_t moveMask, int initialSq, uint16_t flags = 0b0000) {
        while (moveMask) {
            int sq = popLSB(moveMask);
            moves.push_back((flags << 12) | (sq << 6) | initialSq);
        }
    }

    vector<uint16_t> moveGen() {
        vector<uint16_t> moves;

        uint64_t moveMask;
        int from, to, flags, i;

        // pawn moves
        if (whiteToMove) {
            // pawn push
            moveMask = (P << 8) & empty & ~Rank8;
            addMoves(moves, moveMask, -8);
            // double pawn push
            moveMask = ((((P & Rank2) << 8) & empty) << 8) & empty;
            addMoves(moves, moveMask, -16, 0b0001);
            // left capture
            moveMask = ((P & ~FileA) << 7) & black & ~Rank8;
            addMoves(moves, moveMask, -7, 0b0100);
            // right capture
            moveMask = ((P & ~FileH) << 9) & black & ~Rank8;
            addMoves(moves, moveMask, -9, 0b0100);
            // ep left capture
            if (enPassantSquare != -1) {
                moveMask = ((P & ~FileA) << 7) & (1ULL << enPassantSquare);
            } else moveMask = 0;
            addMoves(moves, moveMask, -7, 0b0101);
            // ep right capture
            if (enPassantSquare != -1) {
                moveMask = ((P & ~FileH) << 9) & (1ULL << enPassantSquare);
            } else moveMask = 0;
            addMoves(moves, moveMask, -9, 0b0101);
            // pawn push promotion
            moveMask = (P << 8) & empty & Rank8;
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    moves.push_back((0b1000 << 12) | (sq << 6) | (sq - 8));
                    moves.push_back((0b1001 << 12) | (sq << 6) | (sq - 8));
                    moves.push_back((0b1010 << 12) | (sq << 6) | (sq - 8));
                    moves.push_back((0b1011 << 12) | (sq << 6) | (sq - 8));
                }
            }
            // left capture promotion
            moveMask = ((P & ~FileA) << 7) & black & Rank8;
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    moves.push_back((0b1100 << 12) | (sq << 6) | (sq - 7));
                    moves.push_back((0b1101 << 12) | (sq << 6) | (sq - 7));
                    moves.push_back((0b1110 << 12) | (sq << 6) | (sq - 7));
                    moves.push_back((0b1111 << 12) | (sq << 6) | (sq - 7));
                }
            }
            // right capture promotion
            moveMask = ((P & ~FileH) << 9) & black & Rank8;
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    moves.push_back((0b1100 << 12) | (sq << 6) | (sq - 9));
                    moves.push_back((0b1101 << 12) | (sq << 6) | (sq - 9));
                    moves.push_back((0b1110 << 12) | (sq << 6) | (sq - 9));
                    moves.push_back((0b1111 << 12) | (sq << 6) | (sq - 9));
                }
            }
        }
        else {
            // pawn push
            moveMask = (p >> 8) & empty & ~Rank1;
            addMoves(moves, moveMask, 8);
            // double pawn push
            moveMask = ((((p & Rank7) >> 8) & empty) >> 8) & empty;
            addMoves(moves, moveMask, 16, 0b0001);
            // left capture
            moveMask = ((p & ~FileH) >> 7) & white & ~Rank1;
            addMoves(moves, moveMask, 7, 0b0100);
            // right capture
            moveMask = ((p & ~FileA) >> 9) & white & ~Rank1;
            addMoves(moves, moveMask, 9, 0b0100);
            // ep left capture
            if (enPassantSquare != -1) {
                moveMask = ((p & ~FileH) >> 7) & (1ULL << enPassantSquare);
            } else moveMask = 0;
            addMoves(moves, moveMask, 7, 0b0101);
            // ep right capture
            if (enPassantSquare != -1) {
                moveMask = ((p & ~FileA) >> 9) & (1ULL << enPassantSquare);
            } else moveMask = 0;
            addMoves(moves, moveMask, 9, 0b0101);
            // pawn push promotion
            moveMask = (p >> 8) & empty & Rank1;
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    moves.push_back((0b1000 << 12) | (sq << 6) | (sq + 8));
                    moves.push_back((0b1001 << 12) | (sq << 6) | (sq + 8));
                    moves.push_back((0b1010 << 12) | (sq << 6) | (sq + 8));
                    moves.push_back((0b1011 << 12) | (sq << 6) | (sq + 8));
                }
            }
            // left capture promotion
            moveMask = ((p & ~FileH) >> 7) & white & Rank1;
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    moves.push_back((0b1100 << 12) | (sq << 6) | (sq + 7));
                    moves.push_back((0b1101 << 12) | (sq << 6) | (sq + 7));
                    moves.push_back((0b1110 << 12) | (sq << 6) | (sq + 7));
                    moves.push_back((0b1111 << 12) | (sq << 6) | (sq + 7));
                }
            }
            // right capture promotion
            moveMask = ((p & ~FileA) >> 9) & white & Rank1;
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    moves.push_back((0b1100 << 12) | (sq << 6) | (sq + 9));
                    moves.push_back((0b1101 << 12) | (sq << 6) | (sq + 9));
                    moves.push_back((0b1110 << 12) | (sq << 6) | (sq + 9));
                    moves.push_back((0b1111 << 12) | (sq << 6) | (sq + 9));
                }
            }
        }

        // king moves
        if (whiteToMove) {
            int kingSq = __builtin_ctzll(K);
            uint64_t attacks = kingLookup[kingSq];
            addMovesFromAttackMask(moves, attacks & empty, kingSq);
            addMovesFromAttackMask(moves, attacks & black, kingSq, 0b0100);
        }
        else {
            int kingSq = __builtin_ctzll(k);
            uint64_t attacks = kingLookup[kingSq];
            addMovesFromAttackMask(moves, attacks & empty, kingSq);
            addMovesFromAttackMask(moves, attacks & white, kingSq, 0b0100);
        }

        // castling
        if (whiteToMove) {
            if (cK) {
                if ((empty & 0x60) == 0x60) {
                    uint64_t danger = getDangerSquares();
                    if ((danger & 0x70) == 0) {
                        moves.push_back((0b10 << 12) | (6 << 6) | 4);
                    }
                }
            }
            if (cQ) {
                if ((empty & 0xe) == 0xe) {
                    uint64_t danger = getDangerSquares();
                    if ((danger & 0x1c) == 0) {
                        moves.push_back((0b11 << 12) | (2 << 6) | 4);
                    }
                }
            }
        }
        else {
            if (ck) {
                if ((empty & 0x6000000000000000ULL) == 0x6000000000000000ULL) {
                    uint64_t danger = getDangerSquares();
                    if ((danger & 0x7000000000000000ULL) == 0) {
                        moves.push_back((0b10 << 12) | (62 << 6) | 60);
                    }
                }
            }
            if (cq) {
                if ((empty & 0xe00000000000000ULL) == 0xe00000000000000ULL) {
                    uint64_t danger = getDangerSquares();
                    if ((danger & 0x1c00000000000000) == 0) {
                        moves.push_back((0b11 << 12) | (58 << 6) | 60);
                    }
                }
            }
        }

        // knight moves
        if (whiteToMove) {
            // left-up
            moveMask = ((N & ~(Rank8 | FileA | FileB)) << 6) & (empty | black);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq - 6));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq - 6));
                    }
                }
            }
            // up-left
            moveMask = ((N & ~(FileA | Rank8 | Rank7)) << 15) & (empty | black);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq - 15));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq - 15));
                    }
                }
            }
            // up-right
            moveMask = ((N & ~(FileH | Rank8 | Rank7)) << 17) & (empty | black);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq - 17));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq - 17));
                    }
                }
            }
            // right-up
            moveMask = ((N & ~(Rank8 | FileH | FileG)) << 10) & (empty | black);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq - 10));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq - 10));
                    }
                }
            }
            // right-down
            moveMask = ((N & ~(Rank1 | FileH | FileG)) >> 6) & (empty | black);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq + 6));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq + 6));
                    }
                }
            }
            // down-right
            moveMask = ((N & ~(FileH | Rank1 | Rank2)) >> 15) & (empty | black);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq + 15));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq + 15));
                    }
                }
            }
            // down-left
            moveMask = ((N & ~(FileA | Rank1 | Rank2)) >> 17) & (empty | black);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq + 17));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq + 17));
                    }
                }
            }
            // left-down
            moveMask = ((N & ~(Rank1 | FileA | FileB)) >> 10) & (empty | black);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq + 10));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq + 10));
                    }
                }
            }
        }
        else {
            // left-up
            moveMask = ((n & ~(Rank8 | FileA | FileB)) << 6) & (empty | white);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq - 6));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq - 6));
                    }
                }
            }
            // up-left
            moveMask = ((n & ~(FileA | Rank8 | Rank7)) << 15) & (empty | white);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq - 15));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq - 15));
                    }
                }
            }
            // up-right
            moveMask = ((n & ~(FileH | Rank8 | Rank7)) << 17) & (empty | white);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq - 17));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq - 17));
                    }
                }
            }
            // right-up
            moveMask = ((n & ~(Rank8 | FileH | FileG)) << 10) & (empty | white);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq - 10));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq - 10));
                    }
                }
            }
            // right-down
            moveMask = ((n & ~(Rank1 | FileH | FileG)) >> 6) & (empty | white);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq + 6));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq + 6));
                    }
                }
            }
            // down-right
            moveMask = ((n & ~(FileH | Rank1 | Rank2)) >> 15) & (empty | white);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq + 15));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq + 15));
                    }
                }
            }
            // down-left
            moveMask = ((n & ~(FileA | Rank1 | Rank2)) >> 17) & (empty | white);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq + 17));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq + 17));
                    }
                }
            }
            // left-down
            moveMask = ((n & ~(Rank1 | FileA | FileB)) >> 10) & (empty | white);
            for (int sq = 0; sq < 64; sq++) {
                if (moveMask & (1ULL << sq)) {
                    if (empty & (1ULL << sq)) {
                        moves.push_back((sq << 6) | (sq + 10));
                    }
                    else {
                        moves.push_back((0b0100 << 12) | (sq << 6) | (sq + 10));
                    }
                }
            }
        }

        // rook + queen moves
        if (whiteToMove) {
            // right
            moveMask = (R | Q) & ~FileH;
            i = 0;
            while (moveMask) {
                moveMask <<= 1;
                i++;
                moveMask &= (empty | black);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq - i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq - i));
                        }
                    }
                }
                moveMask &= ~black;
                moveMask &= ~FileH;
            }
            // left
            moveMask = (R | Q) & ~FileA;
            i = 0;
            while (moveMask) {
                moveMask >>= 1;
                i++;
                moveMask &= (empty | black);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq + i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq + i));
                        }
                    }
                }
                moveMask &= ~black;
                moveMask &= ~FileA;
            }
            // up
            moveMask = (R | Q) & ~Rank8;
            i = 0;
            while (moveMask) {
                moveMask <<= 8;
                i += 8;
                moveMask &= (empty | black);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq - i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq - i));
                        }
                    }
                }
                moveMask &= ~black;
                moveMask &= ~Rank8;
            }
            // down
            moveMask = (R | Q) & ~Rank1;
            i = 0;
            while (moveMask) {
                moveMask >>= 8;
                i += 8;
                moveMask &= (empty | black);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq + i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq + i));
                        }
                    }
                }
                moveMask &= ~black;
                moveMask &= ~Rank1;
            }
        }
        else {
            // right
            moveMask = (r | q) & ~FileH;
            i = 0;
            while (moveMask) {
                moveMask <<= 1;
                i++;
                moveMask &= (empty | white);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq - i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq - i));
                        }
                    }
                }
                moveMask &= ~white;
                moveMask &= ~FileH;
            }
            // left
            moveMask = (r | q) & ~FileA;
            i = 0;
            while (moveMask) {
                moveMask >>= 1;
                i++;
                moveMask &= (empty | white);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq + i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq + i));
                        }
                    }
                }
                moveMask &= ~white;
                moveMask &= ~FileA;
            }
            // up
            moveMask = (r | q) & ~Rank8;
            i = 0;
            while (moveMask) {
                moveMask <<= 8;
                i += 8;
                moveMask &= (empty | white);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq - i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq - i));
                        }
                    }
                }
                moveMask &= ~white;
                moveMask &= ~Rank8;
            }
            // down
            moveMask = (r | q) & ~Rank1;
            i = 0;
            while (moveMask) {
                moveMask >>= 8;
                i += 8;
                moveMask &= (empty | white);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq + i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq + i));
                        }
                    }
                }
                moveMask &= ~white;
                moveMask &= ~Rank1;
            }
        }

        // bishop + queen moves
        if (whiteToMove) {
            // up-right
            moveMask = (B | Q) & ~(FileH | Rank8);
            i = 0;
            while (moveMask) {
                moveMask <<= 9;
                i += 9;
                moveMask &= (empty | black);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq - i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq - i));
                        }
                    }
                }
                moveMask &= ~black;
                moveMask &= ~(FileH | Rank8);
            }
            // up-left
            moveMask = (B | Q) & ~(FileA | Rank8);
            i = 0;
            while (moveMask) {
                moveMask <<= 7;
                i += 7;
                moveMask &= (empty | black);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq - i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq - i));
                        }
                    }
                }
                moveMask &= ~black;
                moveMask &= ~(FileA | Rank8);
            }
            // down-right
            moveMask = (B | Q) & ~(FileH | Rank1);
            i = 0;
            while (moveMask) {
                moveMask >>= 7;
                i += 7;
                moveMask &= (empty | black);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq + i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq + i));
                        }
                    }
                }
                moveMask &= ~black;
                moveMask &= ~(FileH | Rank1);
            }
            // down-left
            moveMask = (B | Q) & ~(FileA | Rank1);
            i = 0;
            while (moveMask) {
                moveMask >>= 9;
                i += 9;
                moveMask &= (empty | black);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq + i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq + i));
                        }
                    }
                }
                moveMask &= ~black;
                moveMask &= ~(FileA | Rank1);
            }
        }
        else {
            // up-right
            moveMask = (b | q) & ~(FileH | Rank8);
            i = 0;
            while (moveMask) {
                moveMask <<= 9;
                i += 9;
                moveMask &= (empty | white);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq - i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq - i));
                        }
                    }
                }
                moveMask &= ~white;
                moveMask &= ~(FileH | Rank8);
            }
            // up-left
            moveMask = (b | q) & ~(FileA | Rank8);
            i = 0;
            while (moveMask) {
                moveMask <<= 7;
                i += 7;
                moveMask &= (empty | white);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq - i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq - i));
                        }
                    }
                }
                moveMask &= ~white;
                moveMask &= ~(FileA | Rank8);
            }
            // down-right
            moveMask = (b | q) & ~(FileH | Rank1);
            i = 0;
            while (moveMask) {
                moveMask >>= 7;
                i += 7;
                moveMask &= (empty | white);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq + i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq + i));
                        }
                    }
                }
                moveMask &= ~white;
                moveMask &= ~(FileH | Rank1);
            }
            // down-left
            moveMask = (b | q) & ~(FileA | Rank1);
            i = 0;
            while (moveMask) {
                moveMask >>= 9;
                i += 9;
                moveMask &= (empty | white);
                for (int sq = 0; sq < 64; sq++) {
                    if (moveMask & (1ULL << sq)) {
                        if (empty & (1ULL << sq)) {
                            moves.push_back((sq << 6) | (sq + i));
                        }
                        else {
                            moves.push_back((0b0100 << 12) | (sq << 6) | (sq + i));
                        }
                    }
                }
                moveMask &= ~white;
                moveMask &= ~(FileA | Rank1);
            }
        }

        return moves;
    }

    vector<uint16_t> legalMoveGen() {
        vector<uint16_t> pseudoMoves = moveGen();

        vector<uint16_t> legalMoves;

        for (uint16_t move : pseudoMoves) {
            Board b = *this;
            b.makeMove(move);

            if (b.K && b.k) {
                b.whiteToMove = !b.whiteToMove;
                if (b.whiteToMove) {
                    if ((b.getDangerSquares() & b.K) == 0) {
                        legalMoves.push_back(move);
                    }
                }
                else {
                    if ((b.getDangerSquares() & b.k) == 0) {
                        legalMoves.push_back(move);
                    }
                }
            }
        }

        return legalMoves;
    }

    uint64_t getDangerSquares() {
        uint64_t danger = 0ULL;
        uint64_t moveMask = 0ULL;

        if (whiteToMove) {
            // pawns
            // left capture
            danger |= (p & ~FileH) >> 7;
            // right capture
            danger |= (p & ~FileA) >> 9;

            // king
            // left
            danger |= (k & ~FileH) << 1;
            // right
            danger |= (k & ~FileA) >> 1;
            // down
            danger |= (k & ~Rank8) << 8;
            // up
            danger |= (k & ~Rank1) >> 8;
            // left-down
            danger |= ((k & ~FileH) & ~Rank8) << 9;
            // right-down
            danger |= ((k & ~FileA) & ~Rank8) << 7;
            // left-up
            danger |= ((k & ~FileH) & ~Rank1) >> 7;
            // right-up
            danger |= ((k & ~FileA) & ~Rank1) >> 9;

            // knight
            // left-up
            danger |= (n & ~(Rank8 | FileA | FileB)) << 6;
            // up-left
            danger |= (n & ~(FileA | Rank8 | Rank7)) << 15;
            // up-right
            danger |= (n & ~(FileH | Rank8 | Rank7)) << 17;
            // right-up
            danger |= (n & ~(Rank8 | FileH | FileG)) << 10;
            // right-down
            danger |= (n & ~(Rank1 | FileH | FileG)) >> 6;
            // down-right
            danger |= (n & ~(FileH | Rank1 | Rank2)) >> 15;
            // down-left
            danger |= (n & ~(FileA | Rank1 | Rank2)) >> 17;
            // left-down
            danger |= (n & ~(Rank1 | FileA | FileB)) >> 10;

            // rook + queen
            // right
            moveMask = (r | q) & ~FileH;
            while (moveMask) {
                moveMask <<= 1;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~FileH;
            }
            // left
            moveMask = (r | q) & ~FileA;
            while (moveMask) {
                moveMask >>= 1;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~FileA;
            }
            // up
            moveMask = (r | q) & ~Rank8;
            while (moveMask) {
                moveMask <<= 8;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~Rank8;
            }
            // down
            moveMask = (r | q) & ~Rank1;
            while (moveMask) {
                moveMask >>= 8;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~Rank1;
            }

            // bishop + queen
            // up-right
            moveMask = (b | q) & ~(FileH | Rank8);
            while (moveMask) {
                moveMask <<= 9;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~(FileH | Rank8);
            }
            // up-left
            moveMask = (b | q) & ~(FileA | Rank8);
            while (moveMask) {
                moveMask <<= 7;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~(FileA | Rank8);
            }
            // down-right
            moveMask = (b | q) & ~(FileH | Rank1);
            while (moveMask) {
                moveMask >>= 7;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~(FileH | Rank1);
            }
            // down-left
            moveMask = (b | q) & ~(FileA | Rank1);
            while (moveMask) {
                moveMask >>= 9;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~(FileA | Rank1);
            }
        }
        else {
            // pawns
            // left capture
            danger |= (P & ~FileA) << 7;
            // right capture
            danger |= (P & ~FileH) << 9;

            // king
            // left
            danger |= (K & ~FileH) << 1;
            // right
            danger |= (K & ~FileA) >> 1;
            // down
            danger |= (K & ~Rank8) << 8;
            // up
            danger |= (K & ~Rank1) >> 8;
            // left-down
            danger |= ((K & ~FileH) & ~Rank8) << 9;
            // right-down
            danger |= ((K & ~FileA) & ~Rank8) << 7;
            // left-up
            danger |= ((K & ~FileH) & ~Rank1) >> 7;
            // right-up
            danger |= ((K & ~FileA) & ~Rank1) >> 9;

            // knight
            // left-up
            danger |= (N & ~(Rank8 | FileA | FileB)) << 6;
            // up-left
            danger |= (N & ~(FileA | Rank8 | Rank7)) << 15;
            // up-right
            danger |= (N & ~(FileH | Rank8 | Rank7)) << 17;
            // right-up
            danger |= (N & ~(Rank8 | FileH | FileG)) << 10;
            // right-down
            danger |= (N & ~(Rank1 | FileH | FileG)) >> 6;
            // down-right
            danger |= (N & ~(FileH | Rank1 | Rank2)) >> 15;
            // down-left
            danger |= (N & ~(FileA | Rank1 | Rank2)) >> 17;
            // left-down
            danger |= (N & ~(Rank1 | FileA | FileB)) >> 10;

            // rook + queen
            // right
            moveMask = (R | Q) & ~FileH;
            while (moveMask) {
                moveMask <<= 1;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~FileH;
            }
            // left
            moveMask = (R | Q) & ~FileA;
            while (moveMask) {
                moveMask >>= 1;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~FileA;
            }
            // up
            moveMask = (R | Q) & ~Rank8;
            while (moveMask) {
                moveMask <<= 8;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~Rank8;
            }
            // down
            moveMask = (R | Q) & ~Rank1;
            while (moveMask) {
                moveMask >>= 8;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~Rank1;
            }

            // bishop + queen
            // up-right
            moveMask = (B | Q) & ~(FileH | Rank8);
            while (moveMask) {
                moveMask <<= 9;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~(FileH | Rank8);
            }
            // up-left
            moveMask = (B | Q) & ~(FileA | Rank8);
            while (moveMask) {
                moveMask <<= 7;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~(FileA | Rank8);
            }
            // down-right
            moveMask = (B | Q) & ~(FileH | Rank1);
            while (moveMask) {
                moveMask >>= 7;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~(FileH | Rank1);
            }
            // down-left
            moveMask = (B | Q) & ~(FileA | Rank1);
            while (moveMask) {
                moveMask >>= 9;
                danger |= moveMask;
                moveMask &= empty;
                moveMask &= ~(FileA | Rank1);
            }
        }

        return danger;
    }

    uint64_t perft(int depth) {
        if (depth == 0) return 1;

        uint64_t nodes = 0;

        vector<uint16_t> moves = legalMoveGen();

        for (uint16_t move : moves)
        {
            Board b = *this;
            b.makeMove(move);

            nodes += b.perft(depth - 1);
        }

        return nodes;
    }

    uint64_t perftDivide(int depth) {
        uint64_t nodes = 0;

        vector<uint16_t> moves = legalMoveGen();

        for (uint16_t move : moves)
        {
            Board b = *this;
            b.makeMove(move);

            uint64_t current_nodes = b.perft(depth - 1);
            nodes += current_nodes;

            cout << moveToString(move) << ": " << current_nodes << "\n";
        }

        return nodes;
    }

    string moveToString(uint16_t move) {
        int from = move & 0b111111;
        int to = (move >> 6) & 0b111111;

        return squareToString(from) + squareToString(to);
    }

    string squareToString(int sq) {
        return string(1, 'a' + (sq % 8)) + char('1' + (sq / 8));
    }
};


void perftTestPosition(string name, string pos, uint64_t goal, int depth) {
    Board b;

    b.loadFEN(pos);
    uint64_t nodes = b.perft(depth);
    if (nodes == goal) {
        cout << "Position " << name << ": passed\n"; 
    }
    else {
        cout << "Position " << name << ": failed!!!\nExpected: " << goal << "\nGot: " << nodes << "\n";
    }
}


void perftTimePosition(string name, string pos, int depth) {
    int runs = 5;

    double totalTime = 0.0;
    uint64_t nodes = 0;

    Board b;

    for (int i=0; i<runs; i++) {
        b.loadFEN(pos);

        auto start = chrono::high_resolution_clock::now();
        nodes = b.perft(depth);
        auto stop = chrono::high_resolution_clock::now();

        chrono::duration<double> elapsed = stop - start;
        totalTime += elapsed.count();
    }

    cout << name << " perft " << depth << "\n";
    cout << "Time: " << totalTime / 5 << " s\n";
    cout << "MNPS: " << nodes / (totalTime / 5) / 1e6 << "\n";
}


void perftTest() {
    perftTestPosition("1 (StartPos)", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 119060324, 6);
    perftTestPosition("2 (Kiwipete)", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 193690690, 5);
    perftTestPosition("3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 178633661, 7);
    perftTestPosition("4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 15833292, 5);
    perftTestPosition("5", "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", 15833292, 5);
    perftTestPosition("6", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 89941194, 5);
    perftTestPosition("7", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 164075551, 5);

    cout << "\n";
    perftTimePosition("StartPos", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 6);
    cout << "\n";
    perftTimePosition("Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 5);
    cout << "\n";
}


int main() {

    getKingLookup();

    perftTest();

    return 0;
}