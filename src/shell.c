#define _POSIX_C_SOURCE 200809L
#include "shell.h"

#include "builtins.h"
#include "executor.h"
#include "parser.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Output the primary interactive shell prompt to standard error.
 *
 * Emitting the prompt to stderr ensures that stdout remains unpolluted,
 * which is critical when shell output is redirected or captured by automated test scripts.
 */
static void print_prompt(void) {
    fputs(SHELL_DEFAULT_PROMPT, stderr);
    fflush(stderr);
}

/**
 * @brief Interactive Read-Eval-Print Loop (REPL) driver.
 */
int shell_loop(void) {
    char *line = NULL;
    size_t capacity = 0U;
    int running = 1;

    while (running) {
        /* Non-blocking reap of background processes completed since last prompt */
        reap_background();

        /* Present prompt to user */
        print_prompt();

        errno = 0;
        ssize_t read = getline(&line, &capacity, stdin);
        if (read < 0) {
            if (feof(stdin)) {
                /* End-of-file reached (Ctrl-D); emit newline and cleanly exit */
                fputc('\n', stderr);
                break;
            }
            if (errno == EINTR) {
                /* Interrupted by signal; clear error state and continue prompt loop */
                clearerr(stdin);
                continue;
            }
            /* Unrecoverable stream read failure */
            fprintf(stderr, "shell: input error: %s\n", strerror(errno));
            break;
        }

        /* Strip trailing line delimiters (CR and LF) */
        while (read > 0 && (line[read - 1] == '\n' || line[read - 1] == '\r')) {
            line[--read] = '\0';
        }

        struct Pipeline pipeline;
        char error[256];

        /* Parse command line into pipeline structure with quotes and escapes resolved */
        if (parse_line(line, &pipeline, error, sizeof(error)) != 0) {
            fprintf(stderr, "shell: syntax error: %s\n", error);
            continue;
        }

        /* Blank or whitespace-only lines require no execution */
        if (pipeline.count == 0U) {
            free_pipeline(&pipeline);
            continue;
        }

        /* Execute pipeline commands */
        int result = execute_pipeline(&pipeline);
        if (result == SHELL_STATUS_EXIT) {
            running = 0;
        }

        /* Release all memory allocated for this command cycle */
        free_pipeline(&pipeline);
    }

    /* Release dynamic getline buffer before exiting process */
    free(line);
    return 0;
}
