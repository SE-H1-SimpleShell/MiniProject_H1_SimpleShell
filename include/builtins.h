#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

#define SHELL_STATUS_EXIT 1000

int is_builtin(const char *name);
int run_builtin(const struct Command *cmd, int in_child);

#endif
