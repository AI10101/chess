#pragma once

#include "board.h"
#include "constants.h"
#include "movegen.h"

#include <iostream>


template<bool SideToMove>
inline uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1;

    move moves[256];
    int cnt = moveGen<SideToMove>(board, moves);

    uint64_t nodes = 0;

    for (int i=0; i<cnt; i++) {
        makeMove(moves[i], board);

        int KingSq = __builtin_ctzll(board.pieces[SideToMove*6 + K]);

        if (isSqSafe<SideToMove^1>(board, KingSq)) { // checks if move is legal (king can not be captured in the next move)
            nodes += perft<SideToMove^1>(board, depth - 1);
        }

        unmakeMove(moves[i], board);
    }

    return nodes;
}


inline std::string squareToString(const int sq) {
    return std::string(1, 'a' + (sq % 8)) + char('1' + (sq / 8));
}


inline std::string moveToString(const move m) {
    const int from = m & 0b111111;
    const int to = (m >> 6) & 0b111111;

    return squareToString(from) + squareToString(to);
}


template<bool SideToMove>
uint64_t perftDivide(Board& board, int depth) {
    uint64_t nodes = 0;

    move moves[256];
    int cnt = moveGen<SideToMove>(board, moves);

    for (int i=0; i<cnt; i++) {
        Board next = board;
        makeMove(moves[i], next);

        int KingSq = __builtin_ctzll(next.pieces[SideToMove*6 + K]);

        uint64_t current_nodes = 0;

        if (isSqSafe<SideToMove^1>(next, KingSq)) { // checks if move is legal (king can not be captured in the next move)
            current_nodes += perft<SideToMove^1>(next, depth - 1);
            nodes += current_nodes;
        }

        std::cout << moveToString(moves[i]) << ": " << current_nodes << "\n";
    }

    return nodes;
}


void perftTestPosition(std::string name, std::string fen, uint64_t goal, int depth);


void perftTimePosition(std::string name, std::string fen, int depth);


void perftTest();