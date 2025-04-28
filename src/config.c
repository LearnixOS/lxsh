#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lxsh.h"

void lxsh_init_config(void) {
    char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "lxsh: HOME not set\n");
        return;
    }
    char config_path[256];
    snprintf(config_path, sizeof(config_path), "%s/.lxshrc", home);

    // Create ~/.lxshrc if it doesn't exist
    FILE *file = fopen(config_path, "r");
    if (!file) {
        file = fopen(config_path, "w");
        if (!file) {
            perror("lxsh: cannot create ~/.lxshrc");
            return;
        }
        fprintf(file, "# lxsh configuration file\n");
        fprintf(file, "PS1='\\u@\\h:\\w\\$ '\n");
        fprintf(file, "alias ll='ls -l'\n");
        fprintf(file, "alias la='ls -a'\n");
        fprintf(file, "HOME=%s\n", home);
        fclose(file);
    } else {
        fclose(file);
    }

    // Source ~/.lxshrc
    file = fopen(config_path, "r");
    if (file) {
        char *line;
        while ((line = lxsh_read_line()) != NULL) {
            char **args = lxsh_split_line(line);
            lxsh_execute(args);
            free(line);
            free(args);
        }
        fclose(file);
    }
}
