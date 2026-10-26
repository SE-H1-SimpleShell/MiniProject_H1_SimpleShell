#ifndef SHELL_H
#define SHELL_H

/**
 * @file shell.h
 * @brief Interactive shell REPL (Read-Eval-Print Loop) lifecycle declarations.
 *
 * Coordinates prompt presentation, user input reading, pipeline syntax parsing,
 * execution dispatch, and graceful shutdown handling.
 */

/**
 * @brief Default interactive shell prompt string displayed to stderr.
 */
#define SHELL_DEFAULT_PROMPT "simple-shell$ "

/**
 * @brief Main interactive shell REPL driver loop.
 *
 * Repeatedly displays the command prompt, reaps background zombie processes,
 * reads user input lines via getline(3), parses commands with quote/escape handling,
 * dispatches pipelines for execution, and manages prompt recovery on syntax errors.
 *
 * Terminates cleanly upon:
 *  - Explicit execution of the 'exit' built-in command.
 *  - Receipt of End-Of-File (EOF / Ctrl-D) on standard input.
 *  - Fatal standard input read errors (excluding EINTR).
 *
 * @return 0 upon normal and clean shell termination.
 */
int shell_loop(void);

#endif /* SHELL_H */
