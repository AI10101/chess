#include "constants.h"
#include "perft.h"


int main() {

    getKingLookup();
    getKnightLookup();
    getRookLookup();

    getRookMagic();

    perftTest();

    return 0;
}