#include "board.h"
#include "constants.h"

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