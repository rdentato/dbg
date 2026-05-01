/*
** dbglog: readable formatter for dbg.h logs.
*/

#include <ctype.h>
#include <stdarg.h>
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

typedef struct {
  char **items;
  int count;
  int cap;
} line_list_t;

#define TRK_BUF_SIZE    256

static unsigned char trkbuf[TRK_BUF_SIZE];

typedef struct {
  char *name;
  char *summary;
  line_list_t lines;
} file_block_t;

typedef struct {
  file_block_t *items;
  int count;
  int cap;
} file_list_t;

static int in_test = 0;
static int test_checks = 0;
static int test_failed_checks = 0;
static int file_checks = 0;
static int file_failed_checks = 0;
static char current_file[512] = "";

static int capture_output = 0;
static line_list_t *captured_lines = NULL;

static int in_trk = 0;

static void *xrealloc(void *ptr, size_t size)
{
  void *new_ptr = realloc(ptr, size);

  if (new_ptr == NULL) {
    fputs("dbglog: out of memory\n", stderr);
    exit(3);
  }

  return new_ptr;
}

static char *xstrdup(const char *s)
{
  size_t len = strlen(s) + 1;
  char *copy = xrealloc(NULL, len);

  memcpy(copy, s, len);
  return copy;
}

static void lines_push(line_list_t *lines, const char *text)
{
  if (lines->count == lines->cap) {
    lines->cap = lines->cap ? lines->cap * 2 : 16;
    lines->items = xrealloc(lines->items, sizeof(lines->items[0]) * (size_t)lines->cap);
  }

  lines->items[lines->count++] = xstrdup(text);
}

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

static const char *skip_spaces(const char *s)
{
  while (*s == ' ' || *s == '\t') s++;
  return s;
}

static const char *skip_line_number(const char *s)
{
  const char *p = skip_spaces(s);

  while (isdigit((unsigned char)*p)) p++;
  if (p > s && *p == ' ') return skip_spaces(p);
  return p == s ? p : skip_spaces(s);
}

static int starts_with(const char *s, const char *prefix)
{
  return strncmp(s, prefix, strlen(prefix)) == 0;
}

static void trk_reset(void)
{
  memset(trkbuf, 0, 8);
}

static void check_trk_expectations(void);
static void emit_line(const char *fmt, ...);
static void count_check(int failed);

static void trk_scan_line(const char *line)
{
  int i;

  for (i = 0; i < 8; i++) {
    unsigned int off = trkbuf[i];

    if (off == 0) continue;
    if (trkbuf[off] & 0x80) continue;
    if (strstr(line, (const char *)(trkbuf + off + 1)))
      trkbuf[off] |= 0x80;
  }
}

static int parse_trk_expects(const char *line)
{
  const char *p = skip_spaces(line + 5); /* skip "TRK[:" */
  int n = 0;
  unsigned int pos = 8;

  trk_reset();

  while (*p) {
    while (*p == ' ' || *p == '\t' || *p == ',') p++;
    if (*p == '\0') break;

    if (*p != '"') return 0;
    p++;

    if (*p != '=' && *p != '!') return 0;

    const char *start = p++;  /* points to prefix, then advance */
    const char *end = strchr(p, '"');
    if (!end) return 0;

    size_t len = (size_t)(end - start);  /* includes prefix */

    if (n >= 8) {
      emit_line("      FAIL: (trk: too many expectations)");
      count_check(1);
      return 0;
    }

    if (pos + 1 + len > TRK_BUF_SIZE) {
      emit_line("      FAIL: (trk: buffer overflow)");
      count_check(1);
      return 0;
    }

    trkbuf[n] = (unsigned char)pos;
    memcpy(trkbuf + pos, start, len);
    trkbuf[pos + len] = '\0';

    pos += 1 + (unsigned int)len;
    n++;
    p = end + 1;
  }

  return 1;
}

static void emit_line(const char *fmt, ...)
{
  char line[MAX_LINE + 64];
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(line, sizeof(line), fmt, ap);
  va_end(ap);

  if (capture_output) {
    lines_push(captured_lines, line);
  }
  else {
    printf("%s\n", line);
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

static void finish_file(void)
{
  if (current_file[0] != '\0') {
    emit_line("      RSLT: %d / %d failed", file_failed_checks, file_checks);
  }
}

static void reset_formatter(void)
{
  in_test = 0;
  test_checks = 0;
  test_failed_checks = 0;
  file_checks = 0;
  file_failed_checks = 0;
  current_file[0] = '\0';

  if (in_trk) {
    trk_reset();
    in_trk = 0;
  }
}

static void switch_file(source_t *source)
{
  if (!source->has_location) return;
  if (!strcmp(current_file, source->file)) return;

  finish_file();
  snprintf(current_file, sizeof(current_file), "%s", source->file);
  file_checks = 0;
  file_failed_checks = 0;
  emit_line("      FILE: %s", current_file);
}

static void open_test(void)
{
  if (in_test) {
    fputs("dbglog: nested TST markers are no longer supported\n", stderr);
    emit_line("      TST]: %d / %d failed", test_failed_checks, test_checks);
  }

  test_checks = 0;
  test_failed_checks = 0;
  in_test = 1;
}

static void close_test(void)
{
  if (!in_test) {
    emit_line("      TST]: 0 / 0 failed");
    return;
  }

  emit_line("      TST]: %d / %d failed", test_failed_checks, test_checks);
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

static void check_trk_expectations(void)
{
  int i;

  for (i = 0; i < 8; i++) {
    unsigned int off = trkbuf[i];

    if (off == 0) continue;

    unsigned char c = trkbuf[off];
    int failed = (c == (0x80 | '!')) || (c == '=');

    if (failed)
      emit_line("      FAIL: (%c%s)",
                c & 0x7F, (const char *)(trkbuf + off + 1));
    else
      emit_line("      PASS: (%c%s)",
                c & 0x7F, (const char *)(trkbuf + off + 1));

    count_check(failed);
  }
}

static void process_raw_line(char *line)
{
  source_t source;

  trim_newline(line);
  split_source(line, &source);

  /* --- TRK]: close tracking before emitting the marker ---------- */
  if (!strncmp(line, "TRK]:", 5)) {
    if (in_trk) {
      check_trk_expectations();
      trk_reset();
      in_trk = 0;
    }
    emit_line("      TRK]:");
    return;
  }

  /* --- scan each line inside an open TRK block ------------------ */
  if (in_trk)
    trk_scan_line(line);

  /* --- emit the line -------------------------------------------- */
  if (source.has_location) {
    switch_file(&source);
    emit_line("%5d %s", source.line, line);
  }
  else if (!strncmp(line, "TST]:", 5)) {
    close_test();
  }
  else {
    emit_line("      %s", line);
  }

  /* --- open markers --------------------------------------------- */
  if (!strncmp(line, "TST[:", 5)) {
    open_test();
  }
  else if (!strncmp(line, "TRK[:", 5)) {
    if (in_trk) {
      check_trk_expectations();
      trk_reset();
    }
    in_trk = (parse_trk_expects(line) != 0);
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
    process_raw_line(line);
  }

  if (ferror(fp)) {
    fprintf(stderr, "dbglog: error reading %s\n", name);
    return 0;
  }

  return 1;
}

static int read_stream_lines(FILE *fp, const char *name, line_list_t *lines)
{
  char line[MAX_LINE];

  while (fgets(line, sizeof(line), fp) != NULL) {
    trim_newline(line);
    lines_push(lines, line);
  }

  if (ferror(fp)) {
    fprintf(stderr, "dbglog: error reading %s\n", name);
    return 0;
  }

  return 1;
}

static int is_transformed_lines(line_list_t *lines)
{
  int i;

  for (i = 0; i < lines->count; i++) {
    const char *line = skip_spaces(lines->items[i]);

    if (line[0] == '\0') continue;
    return starts_with(line, "FILE:");
  }

  return 0;
}

static void format_raw_lines(line_list_t *raw, line_list_t *out)
{
  int i;

  capture_output = 1;
  captured_lines = out;
  reset_formatter();

  for (i = 0; i < raw->count; i++) {
    char line[MAX_LINE];

    snprintf(line, sizeof(line), "%s", raw->items[i]);
    process_raw_line(line);
  }

  finish_file();
  capture_output = 0;
  captured_lines = NULL;
}

static file_block_t *files_add(file_list_t *files, const char *name)
{
  file_block_t *file;

  if (files->count == files->cap) {
    files->cap = files->cap ? files->cap * 2 : 8;
    files->items = xrealloc(files->items, sizeof(files->items[0]) * (size_t)files->cap);
  }

  file = &files->items[files->count++];
  file->name = xstrdup(name);
  file->summary = NULL;
  file->lines.items = NULL;
  file->lines.count = 0;
  file->lines.cap = 0;
  return file;
}

static void parse_transformed(line_list_t *lines, file_list_t *files)
{
  file_block_t *current = NULL;
  int i;

  for (i = 0; i < lines->count; i++) {
    const char *trimmed = skip_spaces(lines->items[i]);

    if (starts_with(trimmed, "FILE:")) {
      current = files_add(files, skip_spaces(trimmed + 5));
      continue;
    }

    if (current == NULL) current = files_add(files, "log");

    lines_push(&current->lines, lines->items[i]);
    if (starts_with(trimmed, "RSLT:")) {
      if (current->summary != NULL) free(current->summary);
      current->summary = xstrdup(trimmed);
    }
  }
}

static void html_escape(FILE *fp, const char *s)
{
  for (; *s != '\0'; s++) {
    switch (*s) {
    case '&': fputs("&amp;", fp); break;
    case '<': fputs("&lt;", fp); break;
    case '>': fputs("&gt;", fp); break;
    case '"': fputs("&quot;", fp); break;
    default: fputc(*s, fp); break;
    }
  }
}

static const char *line_class(const char *line)
{
  const char *trimmed = skip_line_number(line);

  if (starts_with(trimmed, "FAIL:")) return "fail";
  if (starts_with(trimmed, "PASS:")) return "pass";
  if (starts_with(trimmed, "EROR:")) return "error";
  if (starts_with(trimmed, "WARN:")) return "warn";
  if (starts_with(trimmed, "INFO:")) return "info";
  if (starts_with(trimmed, "TST[:") || starts_with(trimmed, "TST]:")) return "test";
  if (starts_with(trimmed, "CLK[:") || starts_with(trimmed, "CLK]:")) return "clock";
  if (starts_with(trimmed, "VRB[:") || starts_with(trimmed, "VRB]:")) return "verbose";
  if (starts_with(trimmed, "MTRK:") || starts_with(trimmed, "MCHK:")) return "memory";
  if (starts_with(trimmed, "RSLT:")) return starts_with(trimmed, "RSLT: 0 /") ? "pass" : "fail";
  if (starts_with(trimmed, "TRK[:") || starts_with(trimmed, "TRK]:")) return "trk";
  if (starts_with(trimmed, "`")) return "detail";
  return "plain";
}

static const char *file_class(file_block_t *file)
{
  if (file->summary == NULL) return "plain";
  return starts_with(file->summary, "RSLT: 0 /") ? "pass" : "fail";
}

static void render_html(line_list_t *lines)
{
  file_list_t files = {0};
  int i, j;

  parse_transformed(lines, &files);

  puts("<!doctype html>");
  puts("<html lang=\"en\">\n<head>");
  puts("<meta charset=\"utf-8\">");
  puts("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  puts("<title>dbglog report</title>");
  puts("<style>");
  puts("body{margin:0;font:16px/1.5 system-ui,sans-serif;background:#0f141a;color:#e6edf3}");
  puts("header{position:sticky;top:0;background:#161b22;padding:1rem 1.25rem;border-bottom:1px solid #30363d}");
  puts("h1{margin:0 0 .5rem;font-size:1.25rem}");
  puts("p{margin:.25rem 0 .75rem;color:#9da7b3}");
  puts("nav{display:flex;flex-wrap:wrap;gap:.5rem;margin:.75rem 0}");
  puts("nav a,button{border:1px solid #30363d;background:#21262d;color:#e6edf3;padding:.35rem .65rem;border-radius:.5rem;text-decoration:none;cursor:pointer}");
  puts("main{padding:1rem 1.25rem 2rem}");
  puts("details{margin:0 0 .9rem;border:1px solid #30363d;border-radius:.75rem;background:#161b22;overflow:hidden}");
  puts("summary{list-style:none;cursor:pointer;padding:.8rem 1rem;display:flex;justify-content:space-between;gap:1rem;font-weight:600}");
  puts("summary::-webkit-details-marker{display:none}");
  puts("details.pass summary{background:#0f2d1d}");
  puts("details.fail summary{background:#34161c}");
  puts("pre{margin:0;padding:1rem;overflow:auto;background:#0d1117;font:14px/1.45 ui-monospace,monospace}");
  puts(".line{display:block;white-space:pre}");
  puts(".line.pass{color:#3fb950}.line.fail,.line.error{color:#ff7b72}.line.warn{color:#d29922}.line.info{color:#79c0ff}.line.test{color:#c297ff}.line.clock{color:#a5d6ff}.line.verbose{color:#8ddb8c}.line.memory{color:#ffa657}.line.detail{color:#9da7b3}.line.trk{color:#c297ff}");
  puts(".badge{font-weight:500;color:#9da7b3}.fail .badge{color:#ffb3ad}.pass .badge{color:#8ddb8c}");
  puts("</style>");
  puts("</head>\n<body>");
  puts("<header>");
  puts("<h1>dbglog HTML report</h1>");
  puts("<p>Static report generated from dbg logs. File sections can be expanded or collapsed.</p>");
  puts("<div>");
  puts("<button type=\"button\" onclick=\"for(const d of document.querySelectorAll('details')) d.open=true\">Expand all</button>");
  puts("<button type=\"button\" onclick=\"for(const d of document.querySelectorAll('details')) d.open=false\">Collapse all</button>");
  puts("</div>");
  puts("<nav>");
  for (i = 0; i < files.count; i++) {
    printf("<a href=\"#file-%d\">", i);
    html_escape(stdout, files.items[i].name);
    puts("</a>");
  }
  puts("</nav>");
  puts("</header>");
  puts("<main>");

  for (i = 0; i < files.count; i++) {
    file_block_t *file = &files.items[i];

    printf("<details id=\"file-%d\" class=\"%s\">\n<summary><span>", i, file_class(file));
    html_escape(stdout, file->name);
    fputs("</span>", stdout);
    if (file->summary != NULL) {
      fputs("<span class=\"badge\">", stdout);
      html_escape(stdout, file->summary);
      fputs("</span>", stdout);
    }
    puts("</summary>");
    puts("<pre>");
    for (j = 0; j < file->lines.count; j++) {
      printf("<span class=\"line %s\">", line_class(file->lines.items[j]));
      html_escape(stdout, file->lines.items[j]);
      puts("</span>");
    }
    puts("</pre>");
    puts("</details>");
  }

  puts("</main>\n</body>\n</html>");
}

static void usage(FILE *fp, const char *argv0)
{
  fprintf(fp, "usage: %s [-h] [-H] [-v] [log ...]\n", argv0);
  fprintf(fp, "format dbg.h logs as readable text or HTML\n");
  fprintf(fp, "\n");
  fprintf(fp, "  -h  show this help and exit\n");
  fprintf(fp, "  -H  render a static HTML report\n");
  fprintf(fp, "  -v  show version and exit\n");
  fprintf(fp, "\n");
  fprintf(fp, "  With no log files, dbglog reads from standard input.\n");
}

int main(int argc, char **argv)
{
  int ok = 1;
  int html_mode = 0;
  int first_file = 1;

  while (first_file < argc && argv[first_file][0] == '-') {
    if (!strcmp(argv[first_file], "-h")) {
      usage(stdout, argv[0]);
      return 0;
    }
    if (!strcmp(argv[first_file], "-v")) {
      puts(DBGLOG_VERSION);
      return 0;
    }
    if (!strcmp(argv[first_file], "-H")) {
      html_mode = 1;
      first_file++;
      continue;
    }

    fprintf(stderr, "dbglog: unknown option %s\n", argv[first_file]);
    usage(stderr, argv[0]);
    return 2;
  }

  if (!html_mode) {
    reset_formatter();

    if (first_file == argc) {
      ok = process_stream(stdin, "stdin");
    }
    else {
      int i;

      for (i = first_file; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");

        if (fp == NULL) {
          fprintf(stderr, "dbglog: cannot open %s\n", argv[i]);
          ok = 0;
          continue;
        }
        if (i > first_file) reset_formatter();
        if (!process_stream(fp, argv[i])) ok = 0;
        finish_file();
        fclose(fp);
      }
    }

    if (!ok) return 3;
    if (first_file == argc) finish_file();
    return 0;
  }
  else {
    line_list_t transformed = {0};

    if (first_file == argc) {
      line_list_t input = {0};

      ok = read_stream_lines(stdin, "stdin", &input);
      if (!ok) return 3;

      if (is_transformed_lines(&input)) {
        int i;

        for (i = 0; i < input.count; i++) lines_push(&transformed, input.items[i]);
      }
      else {
        format_raw_lines(&input, &transformed);
      }
    }
    else {
      int i;

      for (i = first_file; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        line_list_t input = {0};

        if (fp == NULL) {
          fprintf(stderr, "dbglog: cannot open %s\n", argv[i]);
          ok = 0;
          continue;
        }

        if (!read_stream_lines(fp, argv[i], &input)) ok = 0;
        fclose(fp);
        if (!ok) continue;

        if (is_transformed_lines(&input)) {
          int j;

          for (j = 0; j < input.count; j++) lines_push(&transformed, input.items[j]);
        }
        else {
          format_raw_lines(&input, &transformed);
        }
      }
    }

    if (!ok) return 3;
    render_html(&transformed);
    return 0;
  }
}
