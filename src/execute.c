#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glob.h>
#include "lxsh.h"

int lxsh_exit_status = 0;
int shell_options = 0;

int lxsh_execute(char **args) {
    if (!args[0]) return 1;

    // Handle variable assignment
    char *equal = strchr(args[0], '=');
    if (equal) {
        *equal = '\0';
        set_variable(args[0], equal + 1);
        *equal = '=';
        return 1;
    }

    // Handle alias definition
    if (strcmp(args[0], "alias") == 0) {
        if (args[1] && strchr(args[1], '=')) {
            char *name = strtok(args[1], "=");
            char *value = strtok(NULL, "");
            if (value) set_alias(name, value);
        }
        return 1;
    }

    // Handle trap (placeholder)
    if (strcmp(args[0], "trap") == 0) {
        printf("lxsh: trap: not fully implemented\n");
        return 1;
    }

    // Expand aliases, variables, and globs
    char **expanded_args = expand_aliases(args);
    char **var_expanded = expand_variables(expanded_args);
    char **glob_expanded = expand_globs(var_expanded);

    // Check for built-ins
    for (int i = 0; i < lxsh_num_builtins(); i++) {
        if (strcmp(glob_expanded[0], builtin_str[i]) == 0) {
            int ret = (*builtin_func[i])(glob_expanded);
            for (int j = 0; glob_expanded[j]; j++) free(glob_expanded[j]);
            free(glob_expanded);
            for (int j = 0; var_expanded[j]; j++) free(var_expanded[j]);
            free(var_expanded);
            for (int j = 0; expanded_args[j]; j++) free(expanded_args[j]);
            free(expanded_args);
            return ret;
        }
    }

    // External command
    int ret = lxsh_launch(glob_expanded);
    for (int j = 0; glob_expanded[j]; j++) free(glob_expanded[j]);
    free(glob_expanded);
    for (int j = 0; var_expanded[j]; j++) free(var_expanded[j]);
    free(var_expanded);
    for (int j = 0; expanded_args[j]; j++) free(expanded_args[j]);
    free(expanded_args);
    if ((shell_options & 1) && lxsh_exit_status != 0) return 0; // set -e
    return ret;
}

int lxsh_launch(char **args) {
    char ***commands = NULL;
    int cmd_count = 0;
    int background = 0;
    int i = 0, pos = 0;

    // Split into piped commands
    while (args[i]) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            cmd_count++;
            commands = realloc(commands, (cmd_count + 1) * sizeof(char**));
            if (!commands) {
                fprintf(stderr, "lxsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
            commands[cmd_count-1] = &args[pos];
            pos = i + 1;
        } else if (strcmp(args[i], "&") == 0 && !args[i+1]) {
            args[i] = NULL;
            background = 1;
            break;
        }
        i++;
    }
    cmd_count++;
    commands = realloc(commands, (cmd_count + 1) * sizeof(char**));
    if (!commands) {
        fprintf(stderr, "lxsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    commands[cmd_count-1] = &args[pos];
    commands[cmd_count] = NULL;

    if (cmd_count == 0) {
        free(commands);
        fprintf(stderr, "lxsh: syntax error\n");
        return 1;
    }

    int pipefd[2 * (cmd_count - 1)];
    pid_t pids[cmd_count];

    // Create pipes
    for (i = 0; i < cmd_count - 1; i++) {
        if (pipe(pipefd + i*2) == -1) {
            perror("lxsh: pipe");
            free(commands);
            return 1;
        }
    }

    // Execute commands
    for (i = 0; i < cmd_count; i++) {
        int out_fd = -1, in_fd = -1;
        char **cmd_args = NULL;
        int arg_pos = 0, j = 0;

        // Handle redirection
        while (commands[i][j]) {
            if (strcmp(commands[i][j], ">") == 0 && commands[i][j+1]) {
                j++;
                out_fd = open(commands[i][j], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (out_fd == -1) {
                    perror("lxsh");
                    if (cmd_args) free(cmd_args);
                    free(commands);
                    return 1;
                }
                j++;
            } else if (strcmp(commands[i][j], "<") == 0 && commands[i][j+1]) {
                j++;
                in_fd = open(commands[i][j], O_RDONLY);
                if (in_fd == -1) {
                    perror("lxsh");
                    if (out_fd != -1) close(out_fd);
                    if (cmd_args) free(cmd_args);
                    free(commands);
                    return 1;
                }
                j++;
            } else {
                if (!cmd_args) {
                    cmd_args = malloc(2 * sizeof(char*));
                    cmd_args[0] = strdup(commands[i][j]);
                    cmd_args[1] = NULL;
                    arg_pos = 1;
                } else {
                    cmd_args = realloc(cmd_args, (arg_pos + 2) * sizeof(char*));
                    cmd_args[arg_pos] = strdup(commands[i][j]);
                    cmd_args[arg_pos+1] = NULL;
                    arg_pos++;
                }
                j++;
            }
        }

        if (!cmd_args) {
            if (out_fd != -1) close(out_fd);
            if (in_fd != -1) close(in_fd);
            fprintf(stderr, "lxsh: syntax error\n");
            if (cmd_args) free(cmd_args);
            free(commands);
            return 1;
        }

        pids[i] = fork();
        if (pids[i] == 0) {
            setpgid(0, 0); // Set process group
            if (i > 0) {
                dup2(pipefd[(i-1)*2], STDIN_FILENO);
            }
            if (i < cmd_count - 1) {
                dup2(pipefd[i*2+1], STDOUT_FILENO);
            }
            for (int k = 0; k < 2 * (cmd_count - 1); k++) {
                close(pipefd[k]);
            }
            if (out_fd != -1) {
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }
            if (in_fd != -1) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            execvp(cmd_args[0], cmd_args);
            perror("lxsh");
            exit(EXIT_FAILURE);
        } else if (pids[i] < 0) {
            perror("lxsh");
        }

        for (j = 0; cmd_args[j]; j++) free(cmd_args[j]);
        free(cmd_args);
        if (out_fd != -1) close(out_fd);
        if (in_fd != -1) close(in_fd);
    }

    // Close pipes
    for (i = 0; i < 2 * (cmd_count - 1); i++) {
        close(pipefd[i]);
    }

    // Wait for processes or add to jobs
    if (!background) {
        for (i = 0; i < cmd_count; i++) {
            int status;
            waitpid(pids[i], &status, 0);
            lxsh_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        }
    } else {
        char *cmd = strdup(args[0]);
        for (i = 0; i < cmd_count; i++) {
            add_job(pids[i], cmd);
        }
        free(cmd);
    }

    free(commands);
    return 1;
}

char *execute_subcommand(char *cmd) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("lxsh: pipe");
        return strdup("");
    }
    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        char **args = lxsh_split_line(cmd);
        lxsh_execute(args);
        exit(0);
    }
    close(pipefd[1]);
    char *output = malloc(LXSH_RL_BUFSIZE);
    if (!output) {
        fprintf(stderr, "lxsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    output[0] = '\0';
    char buffer[256];
    ssize_t n;
    int pos = 0;
    while ((n = read(pipefd[0], buffer, sizeof(buffer)-1)) > 0) {
        buffer[n] = '\0';
        output = realloc(output, pos + n + 1);
        strcpy(output + pos, buffer);
        pos += n;
    }
    close(pipefd[0]);
    waitpid(pid, NULL, 0);
    // Trim trailing newline
    if (pos > 0 && output[pos-1] == '\n') output[pos-1] = '\0';
    return output;
}

char **expand_globs(char **args) {
    char **new_args = malloc(LXSH_TOK_BUFSIZE * sizeof(char*));
    if (!new_args) {
        fprintf(stderr, "lxsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    for (int i = 0; args[i]; i++) {
        glob_t glob_result;
        int flags = (shell_options & 2) ? GLOB_NOCHECK | GLOB_NOSORT : GLOB_NOCHECK;
        if (glob(args[i], flags, NULL, &glob_result) == 0) {
            for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                new_args[pos] = strdup(glob_result.gl_pathv[j]);
                pos++;
            }
            globfree(&glob_result);
        } else {
            new_args[pos] = strdup(args[i]);
            pos++;
        }
    }
    new_args[pos] = NULL;
    return new_args;
}
