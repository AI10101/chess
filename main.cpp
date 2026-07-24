#include "constants.h"
#include "perft.h"


int main() {

    getKingLookup();
    getKnightLookup();
    getRookLookup();

    perftTest();

    return 0;
}