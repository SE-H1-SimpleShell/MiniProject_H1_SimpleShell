#ifndef BUILTINS_H
#define BUILTINS_H

/**
 * @file builtins.h
 * @brief Built-in command declarations, status codes, and dispatch interfaces.
 *
 * Provides execution handlers for internal shell commands that must run within
 * the shell process environment (such as cd and exit) or are optimized for internal
 * execution without external process spawning (pwd, echo, help).
 */

#include "parser.h"

/**
 * @brief Special status code indicating that the shell interactive loop should terminate.
 *
 * Returned by run_builtin when the 'exit' command is executed in the parent shell context.
 */
#define SHELL_STATUS_EXIT 1000

/**
 * @brief Standard built-in execution result codes.
 */
#define SHELL_BUILTIN_SUCCESS    0
#define SHELL_BUILTIN_FAILURE    1
#define SHELL_BUILTIN_USAGE_ERR  2
#define SHELL_BUILTIN_NOT_FOUND  127

/**
 * @brief Check if the given command name corresponds to a supported built-in command.
 *
 * Supported built-in commands:
 *  - cd   : Change working directory
 *  - pwd  : Print current working directory
 *  - echo : Print arguments to standard output
 *  - help : Display built-in usage and supported features
 *  - exit : Terminate the shell session
 *
 * @param name The command name to inspect (may be NULL).
 * @return 1 if name matches a built-in command, 0 otherwise.
 */
int is_builtin(const char *name);

/**
 * @brief Execute a built-in command with argument and context inspection.
 *
 * Dispatches to the appropriate built-in implementation. If in_child is true,
 * commands such as 'exit' will terminate the child process immediately via _Exit(0)
 * rather than propagating the SHELL_STATUS_EXIT sentinel to the parent loop.
 *
 * @param cmd Pointer to the Command structure containing argv and argc.
 * @param in_child Boolean flag indicating whether execution is within a forked child.
 * @return Execution status code (0 on success, SHELL_STATUS_EXIT for exit, >0 on failure).
 */
int run_builtin(const struct Command *cmd, int in_child);

#endif /* BUILTINS_H */
