#define _POSIX_C_SOURCE 200809L
#include "builtins.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Determine if a given command name is a supported built-in command.
 */
int is_builtin(const char *name) {
    if (name == NULL) {
        return 0;
    }
    return (strcmp(name, "cd") == 0 ||
            strcmp(name, "pwd") == 0 ||
            strcmp(name, "echo") == 0 ||
            strcmp(name, "help") == 0 ||
            strcmp(name, "exit") == 0);
}

/**
 * @brief Change current working directory, updating PWD and OLDPWD environment variables.
 */
static int builtin_cd(const struct Command *cmd) {
    const char *target = NULL;

    if (cmd->argc > 2U) {
        fprintf(stderr, "cd: too many arguments\n");
        return SHELL_BUILTIN_USAGE_ERR;
    }

    if (cmd->argc == 1U) {
        target = getenv("HOME");
        if (target == NULL || target[0] == '\0') {
            fprintf(stderr, "cd: HOME is not set\n");
            return SHELL_BUILTIN_FAILURE;
        }
    } else if (strcmp(cmd->argv[1], "-") == 0) {
        target = getenv("OLDPWD");
        if (target == NULL || target[0] == '\0') {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return SHELL_BUILTIN_FAILURE;
        }
        printf("%s\n", target);
    } else {
        target = cmd->argv[1];
    }

    char *current_cwd = getcwd(NULL, 0);

    if (chdir(target) != 0) {
        fprintf(stderr, "cd: %s: %s\n", target, strerror(errno));
        free(current_cwd);
        return SHELL_BUILTIN_FAILURE;
    }

    if (current_cwd != NULL) {
        (void)setenv("OLDPWD", current_cwd, 1);
        free(current_cwd);
    }

    char *new_cwd = getcwd(NULL, 0);
    if (new_cwd != NULL) {
        (void)setenv("PWD", new_cwd, 1);
        free(new_cwd);
    }

    return SHELL_BUILTIN_SUCCESS;
}

/**
 * @brief Print the absolute path of the current working directory.
 */
static int builtin_pwd(void) {
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        fprintf(stderr, "pwd: %s\n", strerror(errno));
        return SHELL_BUILTIN_FAILURE;
    }

    printf("%s\n", cwd);
    free(cwd);
    return SHELL_BUILTIN_SUCCESS;
}

/**
 * @brief Print command-line arguments separated by spaces, followed by a newline.
 */
static int builtin_echo(const struct Command *cmd) {
    for (size_t i = 1U; i < cmd->argc; ++i) {
        if (i > 1U) {
            putchar(' ');
        }
        fputs(cmd->argv[i], stdout);
    }
    putchar('\n');
    return SHELL_BUILTIN_SUCCESS;
}

/**
 * @brief Display help overview and list of supported shell commands.
 */
static int builtin_help(void) {
    puts("Simple Shell - supported commands:");
    puts("  cd [DIR]       Change current directory");
    puts("  pwd            Print current directory");
    puts("  echo [TEXT...] Print text");
    puts("  help           Show this help message");
    puts("  exit           Exit the shell");
    puts("");
    puts("External commands, pipes (|), redirection (<, >, >>),");
    puts("and background execution (&) are supported.");
    return SHELL_BUILTIN_SUCCESS;
}

/**
 * @brief Execute a built-in command in either parent or child context.
 */
int run_builtin(const struct Command *cmd, int in_child) {
    if (cmd == NULL || cmd->argc == 0U || cmd->argv[0] == NULL) {
        return SHELL_BUILTIN_SUCCESS;
    }

    if (strcmp(cmd->argv[0], "cd") == 0) {
        return builtin_cd(cmd);
    }
    if (strcmp(cmd->argv[0], "pwd") == 0) {
        return builtin_pwd();
    }
    if (strcmp(cmd->argv[0], "echo") == 0) {
        return builtin_echo(cmd);
    }
    if (strcmp(cmd->argv[0], "help") == 0) {
        return builtin_help();
    }
    if (strcmp(cmd->argv[0], "exit") == 0) {
        if (in_child) {
            _Exit(0);
        }
        return SHELL_STATUS_EXIT;
    }

    return SHELL_BUILTIN_NOT_FOUND;
}
