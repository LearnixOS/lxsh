#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "lxsh.h"

Job jobs[MAX_JOBS];
int job_count = 0;

void add_job(pid_t pid, char *command) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        jobs[job_count].job_id = job_count + 1;
        jobs[job_count].command = strdup(command);
        jobs[job_count].status = 0; // running
        job_count++;
        printf("[%d] %d\n", jobs[job_count-1].job_id, pid);
    }
}

void update_jobs(void) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].status == 2) continue;
        int status;
        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
        if (result > 0) {
            jobs[i].status = 2; // done
            printf("[%d] Done\t%s\n", jobs[i].job_id, jobs[i].command);
        }
    }
    // Clean up done jobs
    for (int i = job_count - 1; i >= 0; i--) {
        if (jobs[i].status == 2) {
            free(jobs[i].command);
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j+1];
            }
            job_count--;
        }
    }
}

int lxsh_fg(char **args) {
    int job_id = args[1] ? atoi(args[1]) : 1;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id && jobs[i].status != 2) {
            printf("%s\n", jobs[i].command);
            int status;
            waitpid(jobs[i].pid, &status, 0);
            jobs[i].status = 2;
            lxsh_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
            return 1;
        }
    }
    fprintf(stderr, "lxsh: fg: job not found\n");
    return 1;
}

int lxsh_bg(char **args) {
    int job_id = args[1] ? atoi(args[1]) : 1;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id && jobs[i].status != 2) {
            kill(jobs[i].pid, SIGCONT);
            jobs[i].status = 0; // running
            printf("[%d] %s &\n", jobs[i].job_id, jobs[i].command);
            return 1;
        }
    }
    fprintf(stderr, "lxsh: bg: job not found\n");
    return 1;
}

int lxsh_jobs(char **args) {
    (void)args; // Suppress unused parameter warning
    update_jobs();
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].status != 2) {
            printf("[%d] %s\t%s\n", jobs[i].job_id,
                   jobs[i].status == 0 ? "Running" : "Stopped",
                   jobs[i].command);
        }
    }
    return 1;
}
