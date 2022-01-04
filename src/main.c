#include <assert.h>
#include <stdio.h>

#include "tests.h"
#include "attacks.h"
#include "movegen.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "tables.h"
#include "uci.h"

int main(void) {
    init_LUTs();
    //test_movegen();
    //run_perft_tests();
    uci_loop();
    /*
    run_all_tests();
    MS_Stack *move_state_stk = stk_create(256);
    */
    return 0;
}