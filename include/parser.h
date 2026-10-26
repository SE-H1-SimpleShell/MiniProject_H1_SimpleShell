#ifndef PARSER_H
#define PARSER_H

/**
 * @file parser.h
 * @brief Command line parsing, lexical analysis, and syntax validation interface.
 *
 * Defines the AST (Abstract Syntax Tree) structures for commands and pipelines,
 * tokenization limits, and memory lifecycle routines for parsed shell input.
 */

#include <stddef.h>

/**
 * @brief Maximum number of arguments supported per command (excluding program name and NULL terminator).
 */
#define MAX_ARGS 128

/**
 * @brief Maximum number of command stages allowed in a single pipeline.
 */
#define MAX_PIPE_SEGMENTS 32

/**
 * @brief Represents an individual command invocation within a pipeline stage.
 *
 * Holds dynamically allocated argument strings, redirection targets, and flags.
 * Memory allocated for argv elements, input_file, and output_file is owned by the
 * Command structure and released via free_pipeline().
 */
struct Command {
    char *argv[MAX_ARGS + 2]; /**< Program name + up to MAX_ARGS arguments + NULL sentinel */
    size_t argc;              /**< Total count of arguments currently populated in argv */
    char *input_file;         /**< Target file path for input redirection (<), or NULL */
    char *output_file;        /**< Target file path for output redirection (> or >>), or NULL */
    int append_output;        /**< Flag: 1 if output redirection is append mode (>>), 0 if truncate (>) */
};

/**
 * @brief Represents a parsed sequence of piped commands with optional background execution.
 */
struct Pipeline {
    struct Command commands[MAX_PIPE_SEGMENTS]; /**< Array of command stages linked by pipes */
    size_t count;                               /**< Number of active command stages in the pipeline */
    int background;                             /**< Flag: 1 if executed asynchronously (&), 0 if foreground */
};

/**
 * @brief Parse a raw command-line string into a structured Pipeline.
 *
 * Performs tokenization, single/double quote processing, backslash escape handling,
 * redirection operator extraction (<, >, >>), pipeline chaining (|), and background
 * detection (&).
 *
 * If syntax errors or resource limits are encountered, an explanatory message is
 * written to the provided error buffer and non-zero is returned.
 *
 * @param line The null-terminated input string to parse.
 * @param pipeline Pointer to the destination Pipeline structure to populate.
 * @param error Destination buffer for human-readable error diagnostics.
 * @param error_size Maximum capacity of the error diagnostic buffer.
 * @return 0 on success, -1 on syntax error or resource failure.
 */
int parse_line(const char *line, struct Pipeline *pipeline, char *error, size_t error_size);

/**
 * @brief Release all heap memory owned by a Pipeline and reset fields to zero.
 *
 * Frees all argument strings in each command's argv array, input and output redirection
 * file paths, and clears command counts. Safe to call on empty or partially initialized
 * pipelines.
 *
 * @param pipeline Pointer to the Pipeline whose memory should be freed.
 */
void free_pipeline(struct Pipeline *pipeline);

#endif /* PARSER_H */
