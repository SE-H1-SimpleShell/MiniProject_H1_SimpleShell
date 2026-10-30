#define _POSIX_C_SOURCE 200809L
#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Internal token representation produced by the lexer.
 */
struct Token {
    char *text;          /**< Dynamically allocated null-terminated token string */
    int operator_token;  /**< Flag: 1 if token is a shell operator (| < > >> &), 0 if word */
};

/**
 * @brief Dynamic array of lexical tokens.
 */
struct TokenList {
    struct Token *items; /**< Dynamically allocated buffer of Token entries */
    size_t count;        /**< Current count of valid tokens stored */
    size_t capacity;     /**< Total allocated capacity */
};

/**
 * @brief Helper to format error diagnostics safely within the allocated buffer.
 */
static void set_error(char *error, size_t error_size, const char *message) {
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

/**
 * @brief Append a token to the TokenList, resizing geometrically if necessary.
 */
static int token_push(struct TokenList *list, char *text, int operator_token) {
    if (list->count == list->capacity) {
        size_t new_capacity = (list->capacity == 0U) ? 16U : (list->capacity * 2U);
        struct Token *new_items = realloc(list->items, new_capacity * sizeof(*new_items));
        if (new_items == NULL) {
            return -1;
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count].text = text;
    list->items[list->count].operator_token = operator_token;
    list->count++;
    return 0;
}

/**
 * @brief Duplicate a null-terminated string onto the heap using C11 standard functions.
 */
static char *duplicate_string(const char *text) {
    if (text == NULL) {
        return NULL;
    }
    size_t length = strlen(text) + 1U;
    char *copy = malloc(length);
    if (copy != NULL) {
        memcpy(copy, text, length);
    }
    return copy;
}

/**
 * @brief Append a single character to a dynamically expanding string buffer.
 */
static int buffer_append(char **buffer, size_t *length, size_t *capacity, char c) {
    if (*length + 1U >= *capacity) {
        size_t new_capacity = (*capacity == 0U) ? 32U : (*capacity * 2U);
        char *new_buffer = realloc(*buffer, new_capacity);
        if (new_buffer == NULL) {
            return -1;
        }
        *buffer = new_buffer;
        *capacity = new_capacity;
    }
    (*buffer)[(*length)++] = c;
    (*buffer)[*length] = '\0';
    return 0;
}

/**
 * @brief Finalize and push the accumulated character buffer as a word token.
 */
static int flush_word(struct TokenList *tokens, char **buffer, size_t *length, size_t *capacity) {
    /* Only called once a token has started, so an empty buffer represents a valid
       empty argument (such as echo "" or echo ''). */
    char *word = malloc(*length + 1U);
    if (word == NULL) {
        free(*buffer);
        *buffer = NULL;
        *length = 0U;
        *capacity = 0U;
        return -1;
    }
    if (*buffer != NULL) {
        memcpy(word, *buffer, *length + 1U);
    } else {
        word[0] = '\0';
    }
    if (token_push(tokens, word, 0) != 0) {
        free(word);
        free(*buffer);
        *buffer = NULL;
        *length = 0U;
        *capacity = 0U;
        return -1;
    }
    free(*buffer);
    *buffer = NULL;
    *length = 0U;
    *capacity = 0U;
    return 0;
}

/**
 * @brief Push a shell operator string (| < > >> &) as an operator token.
 */
static int push_operator(struct TokenList *tokens, const char *op) {
    char *text = duplicate_string(op);
    if (text == NULL) {
        return -1;
    }
    if (token_push(tokens, text, 1) != 0) {
        free(text);
        return -1;
    }
    return 0;
}

/**
 * @brief Release all resources allocated in a TokenList.
 */
static void free_tokens(struct TokenList *tokens) {
    if (tokens == NULL) {
        return;
    }
    for (size_t i = 0U; i < tokens->count; ++i) {
        free(tokens->items[i].text);
    }
    free(tokens->items);
    tokens->items = NULL;
    tokens->count = 0U;
    tokens->capacity = 0U;
}

/**
 * @brief Perform lexical analysis on an input string to generate a TokenList.
 *
 * Implements finite-state lexical processing supporting:
 *  - Single quotes ('...'): Preserves literal characters exactly without escape interpretation.
 *  - Double quotes ("..."): Allows embedded escaped quotes (\") and backslashes (\\).
 *  - Unquoted backslash (\x): Escapes whitespace, operators, and literal characters.
 *  - Operators: Single-character (| < > &) and multi-character (>>) tokens.
 */
static int lex(const char *line, struct TokenList *tokens, char *error, size_t error_size) {
    size_t i = 0U;
    char quote = '\0';
    char *buffer = NULL;
    size_t length = 0U;
    size_t capacity = 0U;
    int had_token_content = 0;

    while (line[i] != '\0') {
        unsigned char c = (unsigned char)line[i];

        if (quote != '\0') {
            if (c == (unsigned char)quote) {
                quote = '\0';
                ++i;
                had_token_content = 1;
                continue;
            }
            if (quote == '"' && c == '\\' && line[i + 1U] != '\0') {
                unsigned char next = (unsigned char)line[i + 1U];
                /* Within double quotes, backslash preserves special shell characters;
                   for other characters retain the backslash so utilities (e.g. printf)
                   can interpret sequences such as \n. */
                if (next == '"' || next == '\\' || next == '$' || next == '`' || next == '\n') {
                    ++i;
                    if (buffer_append(&buffer, &length, &capacity, line[i]) != 0) {
                        set_error(error, error_size, "memory allocation failure");
                        free(buffer);
                        return -1;
                    }
                    ++i;
                } else {
                    if (buffer_append(&buffer, &length, &capacity, '\\') != 0 ||
                        buffer_append(&buffer, &length, &capacity, (char)next) != 0) {
                        set_error(error, error_size, "memory allocation failure");
                        free(buffer);
                        return -1;
                    }
                    i += 2U;
                }
                had_token_content = 1;
                continue;
            }
            if (buffer_append(&buffer, &length, &capacity, (char)c) != 0) {
                set_error(error, error_size, "memory allocation failure");
                free(buffer);
                return -1;
            }
            ++i;
            had_token_content = 1;
            continue;
        }

        if (isspace(c)) {
            if (had_token_content) {
                if (flush_word(tokens, &buffer, &length, &capacity) != 0) {
                    set_error(error, error_size, "memory allocation failure");
                    free(buffer);
                    return -1;
                }
                had_token_content = 0;
            }
            ++i;
            continue;
        }

        if (c == '\'' || c == '"') {
            quote = (char)c;
            had_token_content = 1;
            ++i;
            continue;
        }

        if (c == '\\') {
            if (line[i + 1U] == '\0') {
                set_error(error, error_size, "trailing escape character");
                free(buffer);
                return -1;
            }
            ++i;
            if (buffer_append(&buffer, &length, &capacity, line[i]) != 0) {
                set_error(error, error_size, "memory allocation failure");
                free(buffer);
                return -1;
            }
            ++i;
            had_token_content = 1;
            continue;
        }

        if (c == '|' || c == '<' || c == '&') {
            if (had_token_content) {
                if (flush_word(tokens, &buffer, &length, &capacity) != 0) {
                    set_error(error, error_size, "memory allocation failure");
                    free(buffer);
                    return -1;
                }
                had_token_content = 0;
            }
            char op[2] = {(char)c, '\0'};
            if (push_operator(tokens, op) != 0) {
                set_error(error, error_size, "memory allocation failure");
                free(buffer);
                return -1;
            }
            ++i;
            continue;
        }

        if (c == '>') {
            if (had_token_content) {
                if (flush_word(tokens, &buffer, &length, &capacity) != 0) {
                    set_error(error, error_size, "memory allocation failure");
                    free(buffer);
                    return -1;
                }
                had_token_content = 0;
            }
            if (line[i + 1U] == '>') {
                if (push_operator(tokens, ">>") != 0) {
                    set_error(error, error_size, "memory allocation failure");
                    free(buffer);
                    return -1;
                }
                i += 2U;
            } else {
                if (push_operator(tokens, ">") != 0) {
                    set_error(error, error_size, "memory allocation failure");
                    free(buffer);
                    return -1;
                }
                ++i;
            }
            continue;
        }

        if (buffer_append(&buffer, &length, &capacity, (char)c) != 0) {
            set_error(error, error_size, "memory allocation failure");
            free(buffer);
            return -1;
        }
        had_token_content = 1;
        ++i;
    }

    if (quote != '\0') {
        set_error(error, error_size, "unterminated quote");
        free(buffer);
        return -1;
    }
    if (had_token_content && flush_word(tokens, &buffer, &length, &capacity) != 0) {
        set_error(error, error_size, "memory allocation failure");
        free(buffer);
        return -1;
    }
    free(buffer);
    return 0;
}

/**
 * @brief Check if a token matches the specified operator string.
 */
static int is_op(const struct Token *token, const char *op) {
    return (token->operator_token != 0 && strcmp(token->text, op) == 0);
}

/**
 * @brief Add an argument to a Command structure with bounds validation.
 */
static int add_arg(struct Command *command, const char *text) {
    /* argv[0] is program name; allow up to MAX_ARGS arguments following it */
    if (command->argc >= MAX_ARGS + 1U) {
        return -1;
    }
    command->argv[command->argc] = duplicate_string(text);
    if (command->argv[command->argc] == NULL) {
        return -1;
    }
    command->argc++;
    command->argv[command->argc] = NULL;
    return 0;
}

/**
 * @brief Free all dynamic memory associated with a Pipeline.
 */
void free_pipeline(struct Pipeline *pipeline) {
    if (pipeline == NULL) {
        return;
    }
    for (size_t i = 0U; i < pipeline->count; ++i) {
        struct Command *cmd = &pipeline->commands[i];
        for (size_t j = 0U; j < cmd->argc; ++j) {
            free(cmd->argv[j]);
            cmd->argv[j] = NULL;
        }
        free(cmd->input_file);
        free(cmd->output_file);
        cmd->input_file = NULL;
        cmd->output_file = NULL;
        cmd->argc = 0U;
        cmd->append_output = 0;
    }
    pipeline->count = 0U;
    pipeline->background = 0;
}

/**
 * @brief Parse a raw command-line string into a structured Pipeline.
 */
int parse_line(const char *line, struct Pipeline *pipeline, char *error, size_t error_size) {
    if (pipeline == NULL || line == NULL) {
        set_error(error, error_size, "invalid parser arguments");
        return -1;
    }

    memset(pipeline, 0, sizeof(*pipeline));
    if (error != NULL && error_size > 0U) {
        error[0] = '\0';
    }

    struct TokenList tokens = {0};
    if (lex(line, &tokens, error, error_size) != 0) {
        free_tokens(&tokens);
        return -1;
    }
    if (tokens.count == 0U) {
        free_tokens(&tokens);
        return 0;
    }

    /* Count only unquoted operators; quoted and escaped ampersands are words. */
    size_t background_count = 0U;
    for (size_t i = 0U; i < tokens.count; ++i) {
        if (is_op(&tokens.items[i], "&")) {
            ++background_count;
        }
    }
    if (background_count > 1U) {
        set_error(error, error_size, "multiple background operators are not supported");
        free_tokens(&tokens);
        return -1;
    }

    pipeline->count = 1U;
    struct Command *current = &pipeline->commands[0];

    for (size_t i = 0U; i < tokens.count; ++i) {
        const struct Token *token = &tokens.items[i];

        if (!token->operator_token) {
            if (add_arg(current, token->text) != 0) {
                set_error(error, error_size, "too many arguments or memory allocation failure");
                free_tokens(&tokens);
                free_pipeline(pipeline);
                return -1;
            }
            continue;
        }

        if (is_op(token, "|")) {
            if (current->argc == 0U) {
                set_error(error, error_size, "empty command before pipe");
                free_tokens(&tokens);
                free_pipeline(pipeline);
                return -1;
            }
            if (pipeline->count >= MAX_PIPE_SEGMENTS) {
                set_error(error, error_size, "pipeline is too long");
                free_tokens(&tokens);
                free_pipeline(pipeline);
                return -1;
            }
            current = &pipeline->commands[pipeline->count++];
            continue;
        }

        if (is_op(token, "<") || is_op(token, ">") || is_op(token, ">>")) {
            if (current->argc == 0U) {
                set_error(error, error_size, "redirection without a command");
                free_tokens(&tokens);
                free_pipeline(pipeline);
                return -1;
            }
            if (i + 1U >= tokens.count || tokens.items[i + 1U].operator_token != 0) {
                set_error(error, error_size, "redirection requires a file name");
                free_tokens(&tokens);
                free_pipeline(pipeline);
                return -1;
            }
            const char *filename = tokens.items[++i].text;
            if (is_op(token, "<")) {
                if (current->input_file != NULL) {
                    set_error(error, error_size, "multiple input redirections are not supported");
                    free_tokens(&tokens);
                    free_pipeline(pipeline);
                    return -1;
                }
                current->input_file = duplicate_string(filename);
            } else {
                if (current->output_file != NULL) {
                    set_error(error, error_size, "multiple output redirections are not supported");
                    free_tokens(&tokens);
                    free_pipeline(pipeline);
                    return -1;
                }
                current->output_file = duplicate_string(filename);
                current->append_output = is_op(token, ">>");
            }
            if ((is_op(token, "<") && current->input_file == NULL) ||
                ((!is_op(token, "<")) && current->output_file == NULL)) {
                set_error(error, error_size, "memory allocation failure");
                free_tokens(&tokens);
                free_pipeline(pipeline);
                return -1;
            }
            continue;
        }

        if (is_op(token, "&")) {
            if (i + 1U != tokens.count) {
                set_error(error, error_size, "background operator must be at the end");
                free_tokens(&tokens);
                free_pipeline(pipeline);
                return -1;
            }
            if (current->argc == 0U) {
                set_error(error, error_size, "background operator without a command");
                free_tokens(&tokens);
                free_pipeline(pipeline);
                return -1;
            }
            pipeline->background = 1;
            continue;
        }

        set_error(error, error_size, "unsupported operator");
        free_tokens(&tokens);
        free_pipeline(pipeline);
        return -1;
    }

    for (size_t i = 0U; i < pipeline->count; ++i) {
        if (pipeline->commands[i].argc == 0U) {
            set_error(error, error_size, "empty command in pipeline");
            free_tokens(&tokens);
            free_pipeline(pipeline);
            return -1;
        }
    }

    free_tokens(&tokens);
    return 0;
}
