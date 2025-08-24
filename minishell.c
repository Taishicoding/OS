/*********************************************************************
Program : miniShell Version : 1.4
--------------------------------------------------------------------
Modified command line interpreter with background process support,
cd command, perror for system calls, and proper exec failure handling
--------------------------------------------------------------------
File : minishell.c
Compiler/System : gcc/linux
Author : Taishi Morgan A1904976
Date :23/08/2025
********************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20  /* max number of command tokens */
#define NL 100 /* input buffer size */

char line[NL]; /* command input buffer */

/*Implemented Structure to handle background jobs*/
struct bg_job {
    pid_t pid;
    int job_id;
    char *command;
    int active;
};
struct bg_job bg_jobs[NV];
int next_job_id = 1;

/*
shell prompt
*/
void prompt(void) {
    fprintf(stdout, "\n msh> ");
    fflush(stdout);
}

/*Initialising the background jobs array to an empty state*/
void init_bg_jobs(void) {
    int i;
    for (i = 0; i < NV; i++) {
        bg_jobs[i].pid = 0;
        bg_jobs[i].job_id = 0;
        bg_jobs[i].command = NULL;
        bg_jobs[i].active = 0;
    }
}

/*Adding a new background job*/
int add_bg_job(pid_t pid, char *command) {
    int i;
    for (i = 0; i < NV; i++) {
        if (!bg_jobs[i].active) {
            bg_jobs[i].pid = pid;
            bg_jobs[i].job_id = next_job_id++;
            bg_jobs[i].command = strdup(command);
            bg_jobs[i].active = 1;
            return bg_jobs[i].job_id;
        }
    }
    return -1;
}

/*Removing the background job*/
void remove_bg_job(pid_t pid) {
    int i;
    for (i = 0; i < NV; i++) {
        if (bg_jobs[i].active && bg_jobs[i].pid == pid) {
            printf("[%d]+ Done                 %s\n", bg_jobs[i].job_id, bg_jobs[i].command);
            fflush(stdout); /* Ensure Done message is visible */
            free(bg_jobs[i].command);
            bg_jobs[i].active = 0;
            bg_jobs[i].pid = 0;
            bg_jobs[i].command = NULL;
            break;
        }
    }
}

/*using WNOHANG, a check is undertaken to find any terminated background processes,
Returning instantanely provided there is no terminated child*/
void check_background_processes(void) {
    int status;
    pid_t pid;
    for (int i = 0; i < NV; i++) {
        if (bg_jobs[i].active) {
            pid = waitpid(bg_jobs[i].pid, &status, WNOHANG);
            if (pid == -1) {
                perror("waitpid");
            } else if (pid > 0) {
                remove_bg_job(pid);
            }
        }
    }
}

/*Reconstructing the command string from the token array*/
void reconstruct_command(char *v[], char *cmd_buf, int token_count) {
    int i;
    cmd_buf[0] = '\0';
    for (i = 0; i < token_count && v[i] != NULL; i++) {
        if (i > 0) strcat(cmd_buf, " ");
        strcat(cmd_buf, v[i]);
    }
}

/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
int main(int argk, char *argv[], char *envp[]) {
    int frkRtnVal; /* value returned by fork sys call */
    char *v[NV];   /* array of pointers to command line tokens */
    char *sep = " \t\n"; /* command line token separators */
    int i;         /* parse index */
    int background; /*Flagging background process detection*/
    char cmd_string[NL]; /*buffer when reconstructing command*/
    int job_id; /*Job id for background processses*/
    /*Initialising background job tracking*/
    init_bg_jobs();
    /* prompt for and process one command line at a time */
    while (1) { /* do Forever */
        /*before each prompt checking background processes*/
        check_background_processes();

        prompt();
        if (fgets(line, NL, stdin) == NULL) {
            perror("fgets");
            if (feof(stdin)) {
                /* Clean up background processes */
                for (i = 0; i < NV; i++) {
                    if (bg_jobs[i].active) {
                        if (kill(bg_jobs[i].pid, SIGTERM) == -1) {
                            perror("kill");
                        }
                        if (waitpid(bg_jobs[i].pid, NULL, 0) == -1) {
                            perror("waitpid");
                        }
                        free(bg_jobs[i].command);
                        bg_jobs[i].active = 0;
                    }
                }
                exit(0);
            }
            continue;
        }
        fflush(stdin);

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') {
            continue; /* to prompt */
        }

        v[0] = strtok(line, sep);
        for (i = 1; i < NV; i++) {
            v[i] = strtok(NULL, sep);
            if (v[i] == NULL) {
                break;
            }
        }

        if (v[0] == NULL) {
            continue;
        }

        /*Checking for the progress Ampersand and removing for arg list*/
        background = 0;
        if (i > 1 && strcmp(v[i - 1], "&") == 0) {
            background = 1;
            v[i - 1] = NULL;
            i--;
        }

        /*reconstructing the command string*/
        if (background) {
            reconstruct_command(v, cmd_string, i);
        }

        /* Handle cd command */
        if (strcmp(v[0], "cd") == 0) {
            char *dir;
            if (v[1] == NULL) {
                /* No argument - use HOME directory */
                dir = getenv("HOME");
                if (dir == NULL) {
                    fprintf(stderr, "cd: HOME not set\n");
                    continue;
                }
            } else {
                /* Use provided directory */
                dir = v[1];
            }
            if (chdir(dir) == -1) {
                perror("cd: chdir failed");
            }
            continue;
        }

        /* fork a child process to exec the command in v[0] */
        switch (frkRtnVal = fork()) {
            case -1: /* fork returns error to parent process */
            {
                perror("fork");
                break;
            }
            case 0: /* code executed only by child process */
            {
                if (execvp(v[0], v) == -1) {
                    perror("execvp");
                    exit(1);
                }
                break;
            }
            default: /* code executed only by parent process */
            {
                if (!background) {
                    if (waitpid(frkRtnVal, NULL, 0) == -1) {
                        perror("waitpid");
                    }
                    printf("%s done \n", v[0]);
                    fflush(stdout);
                } else {
                    job_id = add_bg_job(frkRtnVal, cmd_string);
                    if (job_id != -1) {
                        printf("[%d] %d\n", job_id, frkRtnVal);
                        fflush(stdout); /* Ensure background job output is visible */
                    } else {
                        fprintf(stderr, "Error: No slots available for background job\n");
                    }
                }
                break;
            }
        } /* switch */
    } /* while */
} /* main */