#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define MAX_ARGS 128
#define MAX_PIPE_SEGMENTS 32

struct Command {
    /* Program name + up to MAX_ARGS arguments + NULL terminator. */
    char *argv[MAX_ARGS + 2];
    size_t argc;
    char *input_file;
    char *output_file;
    int append_output;
};

struct Pipeline {
    struct Command commands[MAX_PIPE_SEGMENTS];
    size_t count;
    int background;
};

/* Parse a complete command line. Returns 0 on success, -1 on syntax/error. */
int parse_line(const char *line, struct Pipeline *pipeline, char *error, size_t error_size);

/* Release all heap memory owned by a parsed pipeline. */
void free_pipeline(struct Pipeline *pipeline);

#endif
