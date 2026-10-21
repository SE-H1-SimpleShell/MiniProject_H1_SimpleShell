#include "parser.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(int condition, const char *name) {
    if (condition) {
        printf("PASS: %s\n", name);
    } else {
        printf("FAIL: %s\n", name);
        failures++;
    }
}

int main(void) {
    struct Pipeline p;
    char error[256];

    check(parse_line("echo hello world", &p, error, sizeof(error)) == 0 &&
          p.count == 1 && p.commands[0].argc == 3 &&
          strcmp(p.commands[0].argv[1], "hello") == 0 &&
          strcmp(p.commands[0].argv[2], "world") == 0,
          "basic argument parsing");
    free_pipeline(&p);

    check(parse_line("echo \"hello world\"", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 2 &&
          strcmp(p.commands[0].argv[1], "hello world") == 0,
          "double-quoted argument");
    free_pipeline(&p);

    check(parse_line("printf x | wc -c", &p, error, sizeof(error)) == 0 &&
          p.count == 2 && strcmp(p.commands[1].argv[0], "wc") == 0,
          "pipeline parsing");
    free_pipeline(&p);

    check(parse_line("cat < in.txt > out.txt", &p, error, sizeof(error)) == 0 &&
          strcmp(p.commands[0].input_file, "in.txt") == 0 &&
          strcmp(p.commands[0].output_file, "out.txt") == 0 &&
          p.commands[0].append_output == 0,
          "redirection parsing");
    free_pipeline(&p);

    check(parse_line("echo x >> out.txt", &p, error, sizeof(error)) == 0 &&
          p.commands[0].append_output == 1,
          "append redirection parsing");
    free_pipeline(&p);

    check(parse_line("echo \"unterminated", &p, error, sizeof(error)) != 0,
          "unterminated quote rejected");
    check(parse_line("| ls", &p, error, sizeof(error)) != 0,
          "leading pipe rejected");
    check(parse_line("echo hi |", &p, error, sizeof(error)) != 0,
          "trailing pipe rejected");
    check(parse_line("echo hi & date", &p, error, sizeof(error)) != 0,
          "non-terminal background operator rejected");

    check(parse_line("echo 'a b'", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 2 &&
          strcmp(p.commands[0].argv[1], "a b") == 0,
          "single-quoted argument is literal");
    free_pipeline(&p);

    check(parse_line("echo 'a\\b'", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 2 &&
          strcmp(p.commands[0].argv[1], "a\\b") == 0,
          "single quotes do not process escapes");
    free_pipeline(&p);

    check(parse_line("echo hello\\ world", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 2 &&
          strcmp(p.commands[0].argv[1], "hello world") == 0,
          "backslash outside quotes escapes a space");
    free_pipeline(&p);

    check(parse_line("echo a\\|b", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 2 &&
          strcmp(p.commands[0].argv[1], "a|b") == 0,
          "backslash escapes an operator");
    free_pipeline(&p);

    check(parse_line("echo \"a\\\"b\"", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 2 &&
          strcmp(p.commands[0].argv[1], "a\"b") == 0,
          "double quotes escape an embedded quote");
    free_pipeline(&p);

    check(parse_line("echo \"\"", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 2 &&
          strcmp(p.commands[0].argv[1], "") == 0,
          "empty double-quoted argument preserved");
    free_pipeline(&p);

    check(parse_line("echo ''", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 2 &&
          strcmp(p.commands[0].argv[1], "") == 0,
          "empty single-quoted argument preserved");
    free_pipeline(&p);

    check(parse_line("echo 'unterminated", &p, error, sizeof(error)) != 0,
          "unterminated single quote rejected");
    check(parse_line("echo trailing\\", &p, error, sizeof(error)) != 0,
          "trailing backslash rejected");

    char many[1024];
    size_t used = (size_t)snprintf(many, sizeof(many), "cmd");
    for (int i = 0; i < 128; ++i) {
        used += (size_t)snprintf(many + used, sizeof(many) - used, " a");
    }
    check(parse_line(many, &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == MAX_ARGS + 1U,
          "maximum of 128 arguments accepted");
    free_pipeline(&p);

    used += (size_t)snprintf(many + used, sizeof(many) - used, " a");
    check(parse_line(many, &p, error, sizeof(error)) != 0,
          "129th argument rejected");

    printf("\nParser tests: %d failed\n", failures);
    return failures == 0 ? 0 : 1;
}
