#define _POSIX_C_SOURCE 200809L
#include "builtins.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int is_builtin(const char *name) {
    if (name == NULL) {
        return 0;
    }
    return strcmp(name, "cd") == 0 ||
           strcmp(name, "pwd") == 0 ||
           strcmp(name, "echo") == 0 ||
           strcmp(name, "help") == 0 ||
           strcmp(name, "exit") == 0;
}

static int builtin_cd(const struct Command *cmd) {
    const char *target = NULL;
    if (cmd->argc > 2U) {
        fprintf(stderr, "cd: too many arguments\n");
        return 2;
    }
    if (cmd->argc == 1U) {
        target = getenv("HOME");
        if (target == NULL) {
            fprintf(stderr, "cd: HOME is not set\n");
            return 1;
        }
    } else {
        target = cmd->argv[1];
    }
    if (chdir(target) != 0) {
        fprintf(stderr, "cd: %s: %s\n", target, strerror(errno));
        return 1;
    }
    return 0;
}

static int builtin_pwd(void) {
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        fprintf(stderr, "pwd: %s\n", strerror(errno));
        return 1;
    }
    printf("%s\n", cwd);
    free(cwd);
    return 0;
}

static int builtin_echo(const struct Command *cmd) {
    for (size_t i = 1U; i < cmd->argc; ++i) {
        if (i > 1U) {
            putchar(' ');
        }
        fputs(cmd->argv[i], stdout);
    }
    putchar('\n');
    return 0;
}

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
    return 0;
}

int run_builtin(const struct Command *cmd, int in_child) {
    if (cmd == NULL || cmd->argc == 0U) {
        return 0;
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
        return SHELL_STATUS_EXIT; /* Special value handled by executor. */
    }
    return 127;
}
