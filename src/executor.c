#define _POSIX_C_SOURCE 200809L
#include "executor.h"

#include "builtins.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int apply_redirection(const struct Command *cmd) {
    if (cmd->input_file != NULL) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "shell: cannot open %s: %s\n", cmd->input_file, strerror(errno));
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            fprintf(stderr, "shell: dup2: %s\n", strerror(errno));
            close(fd);
            return -1;
        }
        close(fd);
    }

    if (cmd->output_file != NULL) {
        int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int fd = open(cmd->output_file, flags, 0666);
        if (fd < 0) {
            fprintf(stderr, "shell: cannot open %s: %s\n", cmd->output_file, strerror(errno));
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            fprintf(stderr, "shell: dup2: %s\n", strerror(errno));
            close(fd);
            return -1;
        }
        close(fd);
    }
    return 0;
}

static int run_single_parent_builtin(const struct Command *cmd) {
    int saved_stdin = -1;
    int saved_stdout = -1;

    if (cmd->input_file != NULL) {
        saved_stdin = dup(STDIN_FILENO);
        if (saved_stdin < 0) {
            fprintf(stderr, "shell: dup stdin: %s\n", strerror(errno));
            return 1;
        }
    }
    if (cmd->output_file != NULL) {
        saved_stdout = dup(STDOUT_FILENO);
        if (saved_stdout < 0) {
            fprintf(stderr, "shell: dup stdout: %s\n", strerror(errno));
            if (saved_stdin >= 0) close(saved_stdin);
            return 1;
        }
    }

    if (apply_redirection(cmd) != 0) {
        if (saved_stdin >= 0) { (void)dup2(saved_stdin, STDIN_FILENO); close(saved_stdin); }
        if (saved_stdout >= 0) { (void)dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        return 1;
    }

    int result = run_builtin(cmd, 0);
    /* stdio may be fully buffered when the shell is driven by a script.
       Flush before restoring stdout so redirected built-in output reaches
       the target file instead of being emitted later to the terminal. */
    fflush(stdout);

    if (saved_stdin >= 0) {
        (void)dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout >= 0) {
        (void)dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    return result == SHELL_STATUS_EXIT ? SHELL_STATUS_EXIT : result;
}

static void child_exec(const struct Command *cmd, int in_fd, int out_fd) {
    if (in_fd != STDIN_FILENO && dup2(in_fd, STDIN_FILENO) < 0) {
        fprintf(stderr, "shell: dup2 stdin: %s\n", strerror(errno));
        _exit(126);
    }
    if (out_fd != STDOUT_FILENO && dup2(out_fd, STDOUT_FILENO) < 0) {
        fprintf(stderr, "shell: dup2 stdout: %s\n", strerror(errno));
        _exit(126);
    }
    if (in_fd != STDIN_FILENO) close(in_fd);
    if (out_fd != STDOUT_FILENO) close(out_fd);

    if (apply_redirection(cmd) != 0) {
        _exit(126);
    }

    if (is_builtin(cmd->argv[0])) {
        int rc = run_builtin(cmd, 1);
        _exit(rc == SHELL_STATUS_EXIT ? 0 : rc);
    }

    execvp(cmd->argv[0], cmd->argv);
    fprintf(stderr, "shell: %s: %s\n", cmd->argv[0], strerror(errno));
    _exit(errno == ENOENT ? 127 : 126);
}

static int execute_children(const struct Pipeline *pipeline) {
    pid_t pids[MAX_PIPE_SEGMENTS];
    size_t pid_count = 0U;
    int previous_read = STDIN_FILENO;

    for (size_t i = 0U; i < pipeline->count; ++i) {
        int pipe_fd[2] = {-1, -1};
        if (i + 1U < pipeline->count) {
            if (pipe(pipe_fd) != 0) {
                fprintf(stderr, "shell: pipe: %s\n", strerror(errno));
                if (previous_read != STDIN_FILENO) close(previous_read);
                for (size_t j = 0U; j < pid_count; ++j) {
                    (void)waitpid(pids[j], NULL, 0);
                }
                return 1;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "shell: fork: %s\n", strerror(errno));
            if (pipe_fd[0] >= 0) close(pipe_fd[0]);
            if (pipe_fd[1] >= 0) close(pipe_fd[1]);
            if (previous_read != STDIN_FILENO) close(previous_read);
            for (size_t j = 0U; j < pid_count; ++j) {
                (void)waitpid(pids[j], NULL, 0);
            }
            return 1;
        }

        if (pid == 0) {
            int in_fd = previous_read;
            int out_fd = (i + 1U < pipeline->count) ? pipe_fd[1] : STDOUT_FILENO;

            if (pipe_fd[0] >= 0) close(pipe_fd[0]);
            child_exec(&pipeline->commands[i], in_fd, out_fd);
            _exit(126);
        }

        pids[pid_count++] = pid;
        if (previous_read != STDIN_FILENO) close(previous_read);
        if (pipe_fd[1] >= 0) close(pipe_fd[1]);
        previous_read = (i + 1U < pipeline->count) ? pipe_fd[0] : STDIN_FILENO;
    }

    if (previous_read != STDIN_FILENO) close(previous_read);

    if (pipeline->background) {
        printf("[background pid %ld]\n", (long)pids[pid_count - 1U]);
        fflush(stdout);
        return 0;
    }

    int last_status = 0;
    for (size_t i = 0U; i < pid_count; ++i) {
        int status = 0;
        while (waitpid(pids[i], &status, 0) < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "shell: waitpid: %s\n", strerror(errno));
            return 1;
        }
        if (i + 1U == pid_count) {
            if (WIFEXITED(status)) last_status = WEXITSTATUS(status);
            else if (WIFSIGNALED(status)) last_status = 128 + WTERMSIG(status);
        }
    }
    return last_status;
}

int execute_pipeline(const struct Pipeline *pipeline) {
    if (pipeline == NULL || pipeline->count == 0U) {
        return 0;
    }

    if (pipeline->count == 1U && is_builtin(pipeline->commands[0].argv[0]) && !pipeline->background) {
        return run_single_parent_builtin(&pipeline->commands[0]);
    }

    return execute_children(pipeline);
}

void reap_background(void) {
    for (;;) {
        int status = 0;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) {
            break;
        }
        if (WIFEXITED(status)) {
            fprintf(stderr, "[background pid %ld exited %d]\n", (long)pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "[background pid %ld terminated by signal %d]\n", (long)pid, WTERMSIG(status));
        }
    }
}
