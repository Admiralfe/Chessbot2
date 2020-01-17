#include <assert.h>
#include <stdio.h>

#include "tests.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"

int main() {
    run_all_tests();
    test_castling_rights();
    //test_movegen_pawns();
    return 0;
}