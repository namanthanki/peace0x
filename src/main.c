#include "peace0x/init.h"
#include "peace0x/uci.h"
#include "peace0x/perft.h"
#include <string.h>

int main(int argc, char *argv[]) {
    all_init();

    if (argc > 1) {
        if (strcmp(argv[1], "--perft") == 0 ||
            strcmp(argv[1], "perft") == 0 ||
            strcmp(argv[1], "-p") == 0) {
            return run_perft_suite();
        }
    }

    uci_loop();
    return 0;
}
