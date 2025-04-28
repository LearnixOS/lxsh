#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include "lxsh.h"

char *variables[MAX_VARS];
int var_count = 0;
char *aliases[MAX_ALIASES];
int alias_count = 0;

char **lxsh_split_line(char *line) {
    int bufsize = LXSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    if (!tokens) {
        fprintf(stderr, "lxsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    char *token = strtok(line, LXSH_TOK_DELIM);
    while (token) {
        if (strncmp(token, "$(", 2) == 0 && token[strlen(token)-1] == ')') {
            token[strlen(token)-1] = '\0';
            char *sub_output = execute_subcommand(token + 2);
            char *sub_token = strtok(sub_output, "\n");
            while (sub_token) {
                tokens[position] = sub_token;
                position++;
                if (position >= bufsize) {
                    bufsize += LXSH_TOK_BUFSIZE;
                    tokens = realloc(tokens, bufsize * sizeof(char*));
                    if (!tokens) {
                        fprintf(stderr, "lxsh: allocation error\n");
                        exit(EXIT_FAILURE);
                    }
                }
                sub_token = strtok(NULL, "\n");
            }
            free(sub_output);
        } else {
            tokens[position] = token;
            position++;
            if (position >= bufsize) {
                bufsize += LXSH_TOK_BUFSIZE;
                tokens = realloc(tokens, bufsize * sizeof(char*));
                if (!tokens) {
                    fprintf(stderr, "lxsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        token = strtok(NULL, LXSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void set_variable(char *name, char *value) {
    for (int i = 0; i < var_count; i++) {
        char *var = variables[i];
        char *eq = strchr(var, '=');
        if (eq && strncmp(var, name, eq - var) == 0) {
            free(var);
            variables[i] = malloc(strlen(name) + strlen(value) + 2);
            if (!variables[i]) {
                fprintf(stderr, "lxsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
            sprintf(variables[i], "%s=%s", name, value);
            return;
        }
    }
    if (var_count < MAX_VARS) {
        variables[var_count] = malloc(strlen(name) + strlen(value) + 2);
        if (!variables[var_count]) {
            fprintf(stderr, "lxsh: allocation error\n");
            exit(EXIT_FAILURE);
        }
        sprintf(variables[var_count], "%s=%s", name, value);
        var_count++;
    } else {
        fprintf(stderr, "lxsh: too many variables\n");
    }
}

char *get_variable(char *name) {
    if (strcmp(name, "?") == 0) {
        char *status = malloc(16);
        sprintf(status, "%d", lxsh_exit_status);
        return status;
    }
    for (int i = 0; i < var_count; i++) {
        char *var = variables[i];
        char *eq = strchr(var, '=');
        if (eq && strncmp(var, name, eq - var) == 0) {
            return eq + 1;
        }
    }
    return NULL;
}

char **expand_variables(char **args) {
    char **new_args = malloc(LXSH_TOK_BUFSIZE * sizeof(char*));
    if (!new_args) {
        fprintf(stderr, "lxsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    for (int i = 0; args[i]; i++) {
        char *arg = args[i];
        if (arg[0] == '$' && (isalpha(arg[1]) || arg[1] == '?')) {
            char *name = arg + 1;
            char *value = get_variable(name);
            new_args[pos] = strdup(value ? value : "");
        } else {
            new_args[pos] = strdup(arg);
        }
        pos++;
    }
    new_args[pos] = NULL;
    return new_args;
}

void set_alias(char *name, char *value) {
    // Prevent recursive aliases
    if (strcmp(name, value) == 0) {
        fprintf(stderr, "lxsh: alias: recursive alias ignored\n");
        return;
    }
    for (int i = 0; i < alias_count; i++) {
        char *alias = aliases[i];
        char *eq = strchr(alias, '=');
        if (eq && strncmp(alias, name, eq - alias) == 0) {
            free(alias);
            aliases[i] = malloc(strlen(name) + strlen(value) + 2);
            if (!aliases[i]) {
                fprintf(stderr, "lxsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
            sprintf(aliases[i], "%s=%s", name, value);
            return;
        }
    }
    if (alias_count < MAX_ALIASES) {
        aliases[alias_count] = malloc(strlen(name) + strlen(value) + 2);
        if (!aliases[alias_count]) {
            fprintf(stderr, "lxsh: allocation error\n");
            exit(EXIT_FAILURE);
        }
        sprintf(aliases[alias_count], "%s=%s", name, value);
        alias_count++;
    } else {
        fprintf(stderr, "lxsh: too many aliases\n");
    }
}

char *get_alias(char *name) {
    for (int i = 0; i < alias_count; i++) {
        char *alias = aliases[i];
        char *eq = strchr(alias, '=');
        if (eq && strncmp(alias, name, eq - alias) == 0) {
            return eq + 1;
        }
    }
    return NULL;
}

char **expand_aliases(char **args) {
    char **new_args = malloc(LXSH_TOK_BUFSIZE * sizeof(char*));
    if (!new_args) {
        fprintf(stderr, "lxsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    if (args[0]) {
        char *alias_value = get_alias(args[0]);
        if (alias_value) {
            char *copy = strdup(alias_value);
            char *token = strtok(copy, " ");
            while (token) {
                new_args[pos] = strdup(token);
                pos++;
                token = strtok(NULL, " ");
            }
            free(copy);
            for (int i = 1; args[i]; i++) {
                new_args[pos] = strdup(args[i]);
                pos++;
            }
        } else {
            for (int i = 0; args[i]; i++) {
                new_args[pos] = strdup(args[i]);
                pos++;
            }
        }
    }
    new_args[pos] = NULL;
    return new_args;
}

char *expand_ps1(void) {
    char *ps1 = get_variable("PS1");
    if (!ps1) ps1 = "\\u@\\h:\\w\\$ ";
    char *result = malloc(LXSH_RL_BUFSIZE);
    if (!result) {
        fprintf(stderr, "lxsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    for (int i = 0; ps1[i] && pos < LXSH_RL_BUFSIZE - 256; i++) {
        if (ps1[i] == '\\' && ps1[i+1]) {
            i++;
            switch (ps1[i]) {
                case 'u': {
                    struct passwd *pw = getpwuid(getuid());
                    if (pw) {
                        strncpy(result + pos, pw->pw_name, LXSH_RL_BUFSIZE - pos - 1);
                        pos += strlen(pw->pw_name);
                    }
                    break;
                }
                case 'h': {
                    char hostname[256];
                    gethostname(hostname, 256);
                    strncpy(result + pos, hostname, LXSH_RL_BUFSIZE - pos - 1);
                    pos += strlen(hostname);
                    break;
                }
                case 'w': {
                    char cwd[1024];
                    getcwd(cwd, 1024);
                    char *home = get_variable("HOME");
                    if (!home) home = getenv("HOME");
                    if (home && strncmp(cwd, home, strlen(home)) == 0) {
                        snprintf(result + pos, LXSH_RL_BUFSIZE - pos, "~%s", cwd + strlen(home));
                    } else {
                        strncpy(result + pos, cwd, LXSH_RL_BUFSIZE - pos - 1);
                    }
                    pos += strlen(result + pos);
                    break;
                }
                case '$':
                    result[pos++] = '$';
                    break;
                default:
                    result[pos++] = ps1[i];
            }
        } else {
            result[pos++] = ps1[i];
        }
    }
    result[pos] = '\0';
    return result;
}

void lxsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        update_jobs(); // Check job status
        char *prompt = expand_ps1();
        printf("%s", prompt);
        free(prompt);
        line = lxsh_read_line();
        if (!line) break;
        args = lxsh_split_line(line);
        status = lxsh_execute(args);
        free(line);
        free(args);
    } while (status);
}

void lxsh_run_script(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("lxsh");
        exit(EXIT_FAILURE);
    }
    char *line;
    while ((line = lxsh_read_line()) != NULL) {
        char **args = lxsh_split_line(line);
        int status = lxsh_execute(args);
        free(line);
        free(args);
        if (status == 0) break;
    }
    fclose(file);
    exit(lxsh_exit_status);
}
