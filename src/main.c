#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Added for strcmp
#include <signal.h>
#include "lxsh.h"

// Handle SIGINT (Ctrl+C) to refresh prompt
void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\n");
        fflush(stdout);
    }
}

int main(int argc, char **argv) {
    // Set up signal handling
    signal(SIGINT, signal_handler);

    // Handle --help option
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        printf("Usage: lxsh [options] [script]\n");
        printf("Options:\n");
        printf("  --help    Display this help message and exit\n");
        exit(0);
    }

    // Initialize configuration (~/.lxshrc)
    lxsh_init_config();

    // Run script or interactive shell
    if (argc > 1) {
        lxsh_run_script(argv[1]);
    } else {
        lxsh_loop();
    }

    return lxsh_exit_status;
}
