#pragma once

#include "board.h"
#include "movegen.h"
#include "evaluation.h"
#include <algorithm>


template<bool SideToMove>
int search(Board& board, int depth) {
    if (depth == 0) return evaluate(board);

    if (board.halfmoves == 100) return 0; // 50 move rule

    move moves[256];
    int cnt;
    cnt = moveGen<SideToMove>(board, moves);

    int bestEval = -1e9;
    bool legalMoves = false;

    for (int i=0; i<cnt; i++) {
        Board next = board;
        makeMove(moves[i], next);

        int KingSq = __builtin_ctzll(next.pieces[SideToMove*6 + K]);

        if (isSqSafe<SideToMove^1>(next, KingSq)) { // checks if move is legal (king can not be captured in the next move)
            int evaluation = -search<SideToMove^1>(next, depth - 1);
            bestEval = std::max(bestEval, evaluation);
            legalMoves = true;
        }
    }

    if (!legalMoves) {
        int KingSq = __builtin_ctzll(board.pieces[SideToMove*6 + K]);
        if (!isSqSafe<SideToMove^1>(board, KingSq)) { // checkmate
            return -1e9;
        }
        return 0; // stalemate
    }

    return bestEval;
}


template<bool SideToMove>
move findBestMove(Board& board, int depth) {
    move moves[256];
    int cnt;
    cnt = moveGen<SideToMove>(board, moves);

    int bestEval = -1e9;
    move bestMove = moves[0];

    for (int i=0; i<cnt; i++) {
        Board next = board;
        makeMove(moves[i], next);

        int KingSq = __builtin_ctzll(next.pieces[SideToMove*6 + K]);

        if (isSqSafe<SideToMove^1>(next, KingSq)) { // checks if move is legal (king can not be captured in the next move)
            int evaluation = -search<SideToMove^1>(next, depth - 1);
            if (evaluation > bestEval) {
                bestEval = evaluation;
                bestMove = moves[i];
            }
        }
    }

    return bestMove;
}