#pragma once

#include "board.h"
#include "movegen.h"
#include "evaluation.h"
#include <algorithm>


template<bool SideToMove>
int search(Board& board, int depth, int alpha = -INF, int beta = INF) {
    if (depth == 0) return evaluate(board);

    if (board.halfmoves == 100) return 0; // 50 move rule
    // threefold repetition
    int count = 1;
    for (int i=board.ply-2; i>=board.lastIrreversibleMovePly; i-=2) {
        if (board.hashStack[i] == board.hashStack[board.ply]) {
            if (++count == 3) {
                return 0; // draw by threefold repetition
            }
        }
    }

    move moves[256];
    int cnt = moveGen<SideToMove>(board, moves);

    bool legalMoves = false;

    for (int i=0; i<cnt; i++) {
        makeMove<SideToMove>(moves[i], board);

        int KingSq = __builtin_ctzll(board.pieces[SideToMove ? k : K]);
        if (isSqSafe<SideToMove^1>(board, KingSq)) { // checks if move is legal (king can not be captured in the next move)
            int evaluation = -search<SideToMove^1>(board, depth - 1, -beta, -alpha);
            if (evaluation >= beta) {
                unmakeMove<SideToMove>(moves[i], board);
                return beta;
            }
            alpha = std::max(alpha, evaluation);
            legalMoves = true;
        }

        unmakeMove<SideToMove>(moves[i], board);
    }

    if (!legalMoves) {
        int KingSq = __builtin_ctzll(board.pieces[SideToMove ? k : K]);
        if (!isSqSafe<SideToMove^1>(board, KingSq)) { // checkmate
            return -INF + board.ply;
        }
        return 0; // stalemate
    }

    return alpha;
}


template<bool SideToMove>
move findBestMove(Board& board, int depth, int alpha = -INF, int beta = INF) {
    move moves[256];
    int cnt = moveGen<SideToMove>(board, moves);

    move bestMove = moves[0];

    for (int i=0; i<cnt; i++) {
        makeMove<SideToMove>(moves[i], board);

        int KingSq = __builtin_ctzll(board.pieces[SideToMove ? k : K]);
        if (isSqSafe<SideToMove^1>(board, KingSq)) { // checks if move is legal (king can not be captured in the next move)
            int evaluation = -search<SideToMove^1>(board, depth - 1, -beta, -alpha);
            if (evaluation > beta) {
                unmakeMove<SideToMove>(moves[i], board);
                return bestMove;
            }
            if (evaluation > alpha) {
                alpha = evaluation;
                bestMove = moves[i];
            }
        }

        unmakeMove<SideToMove>(moves[i], board);
    }

    return bestMove;
}