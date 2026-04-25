#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbg.h"

#define CAPTURE_PATH "t_output.capture"

static void emit_output(int *line_info, int *line_warn, int *line_error,
                        int *line_vrb, int *line_test,
                        int *line_pass, int *line_fail)
{
  int value = 2;

  *line_info = __LINE__ + 1;
  dbginf("info message");
  *line_warn = __LINE__ + 1;
  dbgwrn("warning message");
  *line_error = __LINE__ + 1;
  dbgerr("error message");

  *line_vrb = __LINE__ + 1;
  dbgvrb("verbose marker") {
    fprintf(stderr, "program stderr sample\n");
  }

  *line_test = __LINE__ + 1;
  dbgtst("output contract") {
    *line_pass = __LINE__ + 1;
    dbgchk(1, "");
    *line_fail = __LINE__ + 1;
    dbgchk(value == 1, "value=%d", value);
  }
}

static int read_capture(char *buffer, size_t size)
{
  FILE *fp;
  size_t used;

  fp = fopen(CAPTURE_PATH, "rb");
  if (fp == NULL) return 0;

  used = fread(buffer, 1, size - 1, fp);
  if (ferror(fp)) {
    fclose(fp);
    return 0;
  }

  buffer[used] = '\0';
  fclose(fp);
  return 1;
}

int main(void)
{
  char expected[1024];
  char actual[1024];
  int line_info;
  int line_warn;
  int line_error;
  int line_vrb;
  int line_test;
  int line_pass;
  int line_fail;

  if (freopen(CAPTURE_PATH, "w", stderr) == NULL) {
    puts("could not redirect stderr");
    return 1;
  }

  emit_output(&line_info, &line_warn, &line_error, &line_vrb,
              &line_test, &line_pass, &line_fail);

  fflush(stderr);
  fclose(stderr);

  if (!read_capture(actual, sizeof(actual))) {
    puts("could not read captured output");
    remove(CAPTURE_PATH);
    return 1;
  }

  snprintf(expected, sizeof(expected),
           "INFO: info message \x0Ft_output.c:%d\n"
           "WARN: warning message \x0Ft_output.c:%d\n"
           "EROR: error message \x0Ft_output.c:%d\n"
           "VRB[: verbose marker \x0Ft_output.c:%d\n"
           "program stderr sample\n"
           "VRB]:\n"
           "TST[: output contract \x0Ft_output.c:%d\n"
           "PASS: (1) \x0Ft_output.c:%d\n"
           "FAIL: (value == 1) \x0Ft_output.c:%d\n"
           "    ` value=2\n"
           "TST]:\n",
           line_info, line_warn, line_error, line_vrb,
           line_test, line_pass, line_fail);

  remove(CAPTURE_PATH);

  if (strcmp(actual, expected) != 0) {
    puts("unexpected dbg output");
    puts("expected:");
    fputs(expected, stdout);
    puts("actual:");
    fputs(actual, stdout);
    return 1;
  }

  return 0;
}
