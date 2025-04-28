#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lxsh.h"

char *lxsh_read_line(void) {
    char *prompt = expand_ps1();
    char *line = readline(prompt);
    free(prompt);
    if (line && *line) {
        add_history(line);
    }
    return line;
}
