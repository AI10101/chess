#include "constants.h"
#include "perft.h"


int main() {

    getKingLookup();
    getKnightLookup();
    getRookLookup();
    getBishopLookup();

    getRookMagic();
    getBishopMagic();

    perftTest();

    return 0;
}