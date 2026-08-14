#include "evaluation.h"


int evaluate(const Board& board) {
    int whiteEval = countMaterial(board, white);
    int blackEval = countMaterial(board, black);

    int evaluation = whiteEval - blackEval;
    int perspective = (board.sideToMove == white) ? 1 : -1;

    return evaluation * perspective;
}