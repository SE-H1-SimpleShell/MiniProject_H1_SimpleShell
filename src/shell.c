#define _POSIX_C_SOURCE 200809L
#include "shell.h"

#include "builtins.h"
#include "executor.h"
#include "parser.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_prompt(void) {
    fputs("simple-shell$ ", stderr);
    fflush(stderr);
}

int shell_loop(void) {
    char *line = NULL;
    size_t capacity = 0U;
    int running = 1;

    while (running) {
        reap_background();
        print_prompt();

        errno = 0;
        ssize_t read = getline(&line, &capacity, stdin);
        if (read < 0) {
            if (feof(stdin)) {
                fputc('\n', stderr);
                break;
            }
            if (errno == EINTR) {
                clearerr(stdin);
                continue;
            }
            fprintf(stderr, "shell: input error: %s\n", strerror(errno));
            break;
        }

        while (read > 0 && (line[read - 1] == '\n' || line[read - 1] == '\r')) {
            line[--read] = '\0';
        }

        struct Pipeline pipeline;
        char error[256];
        if (parse_line(line, &pipeline, error, sizeof(error)) != 0) {
            fprintf(stderr, "shell: syntax error: %s\n", error);
            continue;
        }
        if (pipeline.count == 0U) {
            free_pipeline(&pipeline);
            continue;
        }

        int result = execute_pipeline(&pipeline);
        if (result == SHELL_STATUS_EXIT) {
            running = 0;
        }
        free_pipeline(&pipeline);
    }

    free(line);
    return 0;
}
