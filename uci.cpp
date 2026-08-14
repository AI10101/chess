#include "board.h"
#include "constants.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"

#include <iostream>
#include <sstream>
#include <string>


std::string moveToUCI(const move m) {
    int from = m & 0b111111;
    int to = (m >> 6) & 0b111111;
    int flags = (m >> 12) & 0b1111;

    std::string uciMove = "";

    uciMove += char('a' + (from % 8));
    uciMove += char('1' + (from / 8));
    uciMove += char('a' + (to % 8));
    uciMove += char('1' + (to / 8));

    if (flags & 0b1000) {
        const char promotion[4] = {'n', 'b', 'r', 'q'};
        uciMove += promotion[flags & 0b11];
    }

    return uciMove;
}


void makeUCImove(const std::string& str, Board& board) {
    int from = (str[0] - 'a') + ((str[1] - '1') * 8);
    int to = (str[2] - 'a') + ((str[3] - '1') * 8);

    move movePart = (to << 6) | from;
    move moveMask = 0b111111111111;

    if (str.length() == 5) {
        moveMask |= 0b1011 << 12;
        if (str[4] == 'n') {
            movePart |= 0b1000 << 12;
        }
        else if (str[4] == 'b') {
            movePart |= 0b1001 << 12;
        }
        else if (str[4] == 'r') {
            movePart |= 0b1010 << 12;
        }
        else if (str[4] == 'q') {
            movePart |= 0b1011 << 12;
        }
    }

    move moves[256];
    int cnt;
    if (board.sideToMove) cnt = moveGen<true>(board, moves);
    else cnt = moveGen<false>(board, moves);

    for (int i=0; i<cnt; i++) {
        if ((moves[i] & moveMask) == movePart) {
            makeMove(moves[i], board);
            return;
        }
    }
}


int main() {

    getRookMagic();
    getBishopMagic();

    Board board;

    std::string cmd;

    while (std::getline(std::cin, cmd)) {
        if (cmd == "uci") {
            std::cout << "id name Crimson Fury\n";
            std::cout << "id author AI10101\n";
            std::cout << "uciok\n";  
        }
        else if (cmd == "isready") {
            std::cout << "readyok\n"; 
        }
        else if (cmd == "ucinewgame") {
            board = Board();
        }
        else if (cmd.rfind("position", 0) == 0) {
            // load position
            std::istringstream iss(cmd);
            std::string token;

            iss >> token; // "position"
            iss >> token; // "startpos" or "fen"

            if (token == "startpos") {
                loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
            }
            else { // "fen"
                std::string fen = "";
                for (int i=0; i<6; i++) {
                    iss >> token;
                    fen += token;
                    if (i < 5) fen += " ";
                }
                loadFEN(fen, board);
            }

            if (iss >> token && token == "moves") {
                while (iss >> token) {
                    makeUCImove(token, board);
                }
            }
        }
        else if (cmd.rfind("go", 0) == 0) {
            move bestMove;
            if (board.sideToMove) bestMove = findBestMove<true>(board, 5);
            else bestMove = findBestMove<false>(board, 5);
            std::cout << "bestmove " << moveToUCI(bestMove) << "\n";
        }
        else if (cmd.rfind("perft", 0) == 0) {
            std::istringstream iss(cmd);
            std::string token;

            iss >> token; // "perft"
            iss >> token; // depth

            if (board.sideToMove) perftDivide<true>(board, std::stoi(token));
            else perftDivide<false>(board, std::stoi(token));
        }
        else if (cmd == "test") {
            perftTest();
        }
        else if (cmd == "quit") {
            break;
        }

        std::cout.flush();
    }

    return 0;
}