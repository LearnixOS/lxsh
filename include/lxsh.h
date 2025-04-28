#ifndef LXSH_H
#define LXSH_H

#include <glob.h>

#define LXSH_RL_BUFSIZE 1024
#define LXSH_TOK_BUFSIZE 64
#define LXSH_TOK_DELIM " \t\r\n\a"
#define MAX_VARS 100
#define MAX_ALIASES 50
#define MAX_JOBS 50

// Job structure for job control
typedef struct {
    pid_t pid;
    int job_id;
    char *command;
    int status; // 0: running, 1: stopped, 2: done
} Job;

extern int lxsh_exit_status;
extern char *variables[MAX_VARS];
extern int var_count;
extern char *aliases[MAX_ALIASES];
extern int alias_count;
extern Job jobs[MAX_JOBS];
extern int job_count;
extern int shell_options;

// Built-in command declarations
extern char *builtin_str[];
extern int (*builtin_func[]) (char **);
int lxsh_num_builtins(void);

// Function declarations
void lxsh_loop(void);
void lxsh_run_script(const char *filename);
void lxsh_init_config(void);
char *lxsh_read_line(void);
char **lxsh_split_line(char *line);
int lxsh_execute(char **args);
int lxsh_launch(char **args);
void set_variable(char *name, char *value);
char *get_variable(char *name);
char **expand_variables(char **args);
char *expand_ps1(void);
void set_alias(char *name, char *value);
char *get_alias(char *name);
char **expand_aliases(char **args);
char **expand_globs(char **args);
char *execute_subcommand(char *cmd);
void add_job(pid_t pid, char *command);
void update_jobs(void);
int lxsh_fg(char **args);
int lxsh_bg(char **args);
int lxsh_jobs(char **args);

// Built-in functions
int lxsh_cd(char **args);
int lxsh_help(char **args);
int lxsh_exit(char **args);
int lxsh_echo(char **args);
int lxsh_set(char **args);
int lxsh_shopt(char **args);

#endif
