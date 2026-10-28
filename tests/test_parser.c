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

    /* --- Basic & Quote Parsing Tests --- */
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

    check(parse_line("", &p, error, sizeof(error)) == 0 &&
          p.count == 0,
          "empty line ignored");
    free_pipeline(&p);

    check(parse_line("   \t  ", &p, error, sizeof(error)) == 0 &&
          p.count == 0,
          "whitespace-only line ignored");
    free_pipeline(&p);

    /* --- Argument Limit Checks --- */
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

    /* --- Extended Edge Case & Sanitizer Validation --- */
    check(parse_line(NULL, &p, error, sizeof(error)) != 0,
          "NULL line parameter safely rejected");
    check(parse_line("echo test", NULL, error, sizeof(error)) != 0,
          "NULL pipeline parameter safely rejected");

    check(parse_line("cat < in1.txt < in2.txt", &p, error, sizeof(error)) != 0,
          "multiple input redirections rejected");
    check(parse_line("echo a > out1.txt > out2.txt", &p, error, sizeof(error)) != 0,
          "multiple output redirections rejected");
    check(parse_line("echo a >> out1.txt >> out2.txt", &p, error, sizeof(error)) != 0,
          "multiple append redirections rejected");
    check(parse_line("echo a > out1.txt >> out2.txt", &p, error, sizeof(error)) != 0,
          "overwrite followed by append redirection rejected");
    check(parse_line("echo a >> out1.txt > out2.txt", &p, error, sizeof(error)) != 0,
          "append followed by overwrite redirection rejected");
    check(parse_line("cat > out.txt < in.txt", &p, error, sizeof(error)) == 0 &&
          strcmp(p.commands[0].input_file, "in.txt") == 0 &&
          strcmp(p.commands[0].output_file, "out.txt") == 0 &&
          p.commands[0].append_output == 0,
          "reversed order redirection (output then input) parsed correctly");
    free_pipeline(&p);
    check(parse_line("cmd & &", &p, error, sizeof(error)) != 0,
          "multiple background operators rejected");

    check(parse_line("cat <", &p, error, sizeof(error)) != 0,
          "dangling input redirection rejected");
    check(parse_line("echo hi >", &p, error, sizeof(error)) != 0,
          "dangling output redirection rejected");
    check(parse_line("echo hi >>", &p, error, sizeof(error)) != 0,
          "dangling append redirection rejected");

    check(parse_line("cmd1 | | cmd2", &p, error, sizeof(error)) != 0,
          "consecutive pipes rejected");
    check(parse_line("cmd1 || cmd2", &p, error, sizeof(error)) != 0,
          "double pipe operator rejected");

    check(parse_line("echo a | grep a | wc -l", &p, error, sizeof(error)) == 0 &&
          p.count == 3 &&
          strcmp(p.commands[0].argv[0], "echo") == 0 &&
          strcmp(p.commands[1].argv[0], "grep") == 0 &&
          strcmp(p.commands[2].argv[0], "wc") == 0,
          "three-stage pipeline parsed successfully");
    free_pipeline(&p);

    check(parse_line("echo \"it's fine\" '\"hello\"'", &p, error, sizeof(error)) == 0 &&
          p.commands[0].argc == 3 &&
          strcmp(p.commands[0].argv[1], "it's fine") == 0 &&
          strcmp(p.commands[0].argv[2], "\"hello\"") == 0,
          "mixed quote embedding parsed correctly");
    free_pipeline(&p);

    /* Stress test memory lifecycle for AddressSanitizer leak detection */
    int lifecycle_ok = 1;
    for (int cycle = 0; cycle < 50; ++cycle) {
        if (parse_line("cmd arg1 \"arg 2\" 'arg 3' < in.txt > out.txt | stage2 &",
                       &p, error, sizeof(error)) != 0 || p.count != 2) {
            lifecycle_ok = 0;
            break;
        }
        free_pipeline(&p);
    }
    check(lifecycle_ok, "repeated parse-and-free lifecycle leak-free under ASan");

    printf("\nParser tests: %d failed\n", failures);
    return failures == 0 ? 0 : 1;
}
