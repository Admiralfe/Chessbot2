#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <cargs.h>

#include "tests.h"
#include "tables.h"
#include "uci.h"

static struct cag_option options[] = {
    {
     .identifier = 'p',
     .access_letters = "p",
     .access_name = "perft",
     .value_name = "DEPTH",
     .description = "Run perft tests"
    },
    {
     .identifier = 'u',
     .access_letters = "u",
     .access_name = "uci",
     .value_name = NULL,
     .description = "Run in UCI mode"
    }
};

struct config_opts {
    bool run_perft;
    int perft_depth;
    bool uci_mode;
};

int main(int argc, char* argv[]) {
    struct config_opts config = {0};
    cag_option_context ctx;
    cag_option_prepare(&ctx, options, CAG_ARRAY_SIZE(options), argc, argv);
    while (cag_option_fetch(&ctx)) {
        char ident = cag_option_get(&ctx);
        switch (ident) {
            case 'p':
                config.run_perft = true;
                config.perft_depth = atoi(cag_option_get_value(&ctx));
                assert(config.perft_depth < 6);
                break;
            case 'u':
                config.uci_mode = true;
                break;
        }
    }
    init_LUTs();
    //test_movegen();
    if (config.run_perft) {
        run_perft_tests(config.perft_depth);
    }
    if (config.uci_mode) {
        uci_loop();
    }
    //run_all_tests();
    return 0;
}
