#include "perft.h"

#include "constants.h"
#include "movegen.h"

#include <cstdint>
#include <iostream>
#include <chrono>


uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1;

    move moves[256];
    int cnt = moveGen(board, moves);

    uint64_t nodes = 0;

    for (int i=0; i<cnt; i++) {
        Board next = board;
        makeMove(moves[i], next);

        int us = next.whiteToMove;
        int them = us ^ 1;

        int enemyKingSq = __builtin_ctzll(next.pieces[them*6]);

        if (isKingSafe(next, enemyKingSq)) {
            nodes += perft(next, depth - 1);
        }
    }

    return nodes;
}


void perftTestPosition(std::string name, std::string fen, uint64_t goal, int depth) {
    Board board;

    loadFEN(fen, board);
    uint64_t nodes = perft(board, depth);
    if (nodes == goal) {
        std::cout << "Position " << name << ": passed\n"; 
    }
    else {
        std::cout << "Position " << name << ": failed!!!\nExpected: " << goal << "\nGot: " << nodes << "\n";
    }
}


void perftTimePosition(std::string name, std::string fen, int depth) {
    int runs = 5;

    double totalTime = 0.0;
    uint64_t nodes = 0;

    Board board;

    for (int i=0; i<runs; i++) {
        loadFEN(fen, board);

        auto start = std::chrono::high_resolution_clock::now();
        nodes = perft(board, depth);
        auto stop = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = stop - start;
        totalTime += elapsed.count();
    }

    std::cout << name << " perft " << depth << "\n";
    std::cout << "Time: " << totalTime / 5 << " s\n";
    std::cout << "MNPS: " << nodes / (totalTime / 5) / 1e6 << "\n";
}


void perftTest() {
    perftTestPosition("1 (StartPos)", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 119060324, 6);
    perftTestPosition("2 (Kiwipete)", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 193690690, 5);
    perftTestPosition("3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 178633661, 7);
    perftTestPosition("4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 15833292, 5);
    perftTestPosition("5", "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", 15833292, 5);
    perftTestPosition("6", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 89941194, 5);
    perftTestPosition("7", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 164075551, 5);

    std::cout << "\n";
    perftTimePosition("StartPos", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 6);
    std::cout << "\n";
    perftTimePosition("Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 5);
    std::cout << "\n";
}