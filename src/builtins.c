#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lxsh.h"

// Built-in command names and functions
char *builtin_str[] = {
    "cd", "help", "exit", "echo", "set", "shopt", "fg", "bg", "jobs"
};

int (*builtin_func[]) (char **) = {
    &lxsh_cd, &lxsh_help, &lxsh_exit, &lxsh_echo, &lxsh_set, &lxsh_shopt,
    &lxsh_fg, &lxsh_bg, &lxsh_jobs
};

int lxsh_num_builtins(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

int lxsh_cd(char **args) {
    char *target = args[1] ? args[1] : get_variable("HOME");
    if (!target) target = getenv("HOME");
    if (!target) {
        fprintf(stderr, "lxsh: cd: HOME not set\n");
        return 1;
    }
    if (chdir(target) != 0) {
        perror("lxsh");
    }
    return 1;
}

int lxsh_help(char **args) {
    (void)args; // Suppress unused parameter warning
    printf("lxsh: A Simple Shell\n");
    printf("Built-in commands:\n");
    for (int i = 0; i < lxsh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    printf("Use man for other programs.\n");
    return 1;
}

int lxsh_exit(char **args) {
    if (args[1]) {
        lxsh_exit_status = atoi(args[1]);
    }
    return 0;
}

int lxsh_echo(char **args) {
    for (int i = 1; args[i]; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    return 1;
}

int lxsh_set(char **args) {
    if (!args[1]) {
        for (int i = 0; i < var_count; i++) {
            printf("%s\n", variables[i]);
        }
    } else if (strcmp(args[1], "-e") == 0) {
        shell_options |= 1; // Enable exit on error
    }
    return 1;
}

int lxsh_shopt(char **args) {
    if (!args[1]) {
        printf("nocaseglob: %s\n", (shell_options & 2) ? "on" : "off");
    } else if (strcmp(args[1], "nocaseglob") == 0) {
        if (args[2] && strcmp(args[2], "-s") == 0) {
            shell_options |= 2; // Enable nocaseglob
        } else if (args[2] && strcmp(args[2], "-u") == 0) {
            shell_options &= ~2; // Disable nocaseglob
        }
    } else {
        printf("lxsh: shopt: invalid option\n");
    }
    return 1;
}
