/*
** dbglog: readable formatter for dbg.h logs.
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DBGLOG_VERSION "dbglog 0.1"
#define MAX_LINE 8192

typedef struct {
  int has_location;
  int line;
  char file[512];
} source_t;

static int in_test = 0;
static int test_checks = 0;
static int test_failed_checks = 0;
static int file_checks = 0;
static int file_failed_checks = 0;
static char current_file[512] = "";

static void trim_newline(char *s)
{
  size_t len = strlen(s);

  while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
    s[--len] = '\0';
  }
}

static void trim_right(char *s)
{
  size_t len = strlen(s);

  while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t')) {
    s[--len] = '\0';
  }
}

static int parse_location(const char *s, source_t *source)
{
  const char *colon = strrchr(s, ':');
  char *end;
  long line;
  size_t file_len;

  if (colon == NULL || colon == s || !isdigit((unsigned char)colon[1])) return 0;

  line = strtol(colon + 1, &end, 10);
  if (*end != '\0' || line <= 0) return 0;

  file_len = (size_t)(colon - s);
  if (file_len >= sizeof(source->file)) file_len = sizeof(source->file) - 1;

  memcpy(source->file, s, file_len);
  source->file[file_len] = '\0';
  source->line = (int)line;
  source->has_location = 1;
  return 1;
}

static void split_source(char *line, source_t *source)
{
  char *sep;
  char *space;

  source->has_location = 0;
  source->line = 0;
  source->file[0] = '\0';

  sep = strchr(line, '\x0f');
  if (sep != NULL) {
    *sep++ = '\0';
    trim_right(line);
    parse_location(sep, source);
    return;
  }

  /* Accept pasted logs where the non-printing separator was lost. */
  space = strrchr(line, ' ');
  if (space != NULL && parse_location(space + 1, source)) {
    *space = '\0';
    trim_right(line);
  }
}

static void switch_file(source_t *source)
{
  if (!source->has_location) return;
  if (!strcmp(current_file, source->file)) return;

  if (current_file[0] != '\0') {
    printf("      RSLT: %d / %d failed\n", file_failed_checks, file_checks);
  }
  snprintf(current_file, sizeof(current_file), "%s", source->file);
  file_checks = 0;
  file_failed_checks = 0;
  printf("      FILE: %s\n", current_file);
}

static void open_test(void)
{
  if (in_test) {
    fprintf(stderr, "dbglog: nested TST markers are no longer supported\n");
    printf("      TST]: %d / %d failed\n", test_failed_checks, test_checks);
  }

  test_checks = 0;
  test_failed_checks = 0;
  in_test = 1;
}

static void close_test(void)
{
  if (!in_test) {
    printf("      TST]: 0 / 0 failed\n");
    return;
  }

  printf("      TST]: %d / %d failed\n", test_failed_checks, test_checks);
  in_test = 0;
}

static void count_check(int failed)
{
  if (!in_test) return;

  test_checks++;
  file_checks++;
  if (failed) {
    test_failed_checks++;
    file_failed_checks++;
  }
}

static void process_line(char *line)
{
  source_t source;

  trim_newline(line);
  split_source(line, &source);

  if (source.has_location) {
    switch_file(&source);
    printf("%5d %s\n", source.line, line);
  }
  else if (!strncmp(line, "TST]:", 5)) {
    close_test();
  }
  else {
    printf("      %s\n", line);
  }

  if (!strncmp(line, "TST[:", 5)) {
    open_test();
  }
  else if (!strncmp(line, "PASS:", 5)) {
    count_check(0);
  }
  else if (!strncmp(line, "FAIL:", 5)) {
    count_check(1);
  }
}

static int process_stream(FILE *fp, const char *name)
{
  char line[MAX_LINE];

  while (fgets(line, sizeof(line), fp) != NULL) {
    process_line(line);
  }

  if (ferror(fp)) {
    fprintf(stderr, "dbglog: error reading %s\n", name);
    return 0;
  }

  return 1;
}

static void usage(FILE *fp, const char *argv0)
{
  fprintf(fp, "usage: %s [log ...]\n", argv0);
  fprintf(fp, "format dbg.h logs with file headers, line numbers, and test summaries\n");
}

int main(int argc, char **argv)
{
  int ok = 1;

  if (argc > 1 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
    usage(stdout, argv[0]);
    return 0;
  }

  if (argc > 1 && !strcmp(argv[1], "--version")) {
    puts(DBGLOG_VERSION);
    return 0;
  }

  if (argc == 1) {
    ok = process_stream(stdin, "stdin");
  }
  else {
    int i;

    for (i = 1; i < argc; i++) {
      FILE *fp = fopen(argv[i], "r");

      if (fp == NULL) {
        fprintf(stderr, "dbglog: cannot open %s\n", argv[i]);
        ok = 0;
        continue;
      }
      if (!process_stream(fp, argv[i])) ok = 0;
      fclose(fp);
    }
  }

  if (!ok) return 3;

  if (current_file[0] != '\0') {
    printf("      RSLT: %d / %d failed\n", file_failed_checks, file_checks);
  }
  return 0;
}
