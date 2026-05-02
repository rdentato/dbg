/*
** dbglog: readable formatter for dbg.h logs.
** Parses single-byte event codes. Validates M? checks. Evaluates dbgtrk.
*/

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define DBGLOG_PRINTF(a,b) __attribute__((format(printf,a,b)))
#else
#define DBGLOG_PRINTF(a,b)
#endif

#define DBGLOG_VERSION "dbglog 2.0"
#define MAX_LINE        8192
#define MAX_ALLOCS      4096
#define MAX_TRK_EXPECTS 8

/* ----------------------------------------------------------------
 * allocation tracking
 * ---------------------------------------------------------------- */

typedef struct {
  uintptr_t ptr;
  size_t    size;
} alloc_t;

static alloc_t allocs[MAX_ALLOCS];
static int     alloc_count;

static int  html_mode;
static int  fail_only;
static int  in_verbatim;
static int  in_trk;
static int  alloc_limit_reported;

static void diag(const char *fmt, ...) DBGLOG_PRINTF(1,2);

static void diag(const char *fmt, ...)
{
  va_list ap;
  fputs("dbglog: ", stderr);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
}

static void alloc_add(uintptr_t ptr, size_t size)
{
  int i;

  if (ptr == 0) return;

  for (i = 0; i < alloc_count; i++) {
    if (allocs[i].ptr == ptr) {
      allocs[i].size = size;
      return;
    }
  }

  if (alloc_count >= MAX_ALLOCS) {
    if (!alloc_limit_reported) {
      diag("allocation table full; further allocations ignored");
      alloc_limit_reported = 1;
    }
    return;
  }

  allocs[alloc_count].ptr  = ptr;
  allocs[alloc_count].size = size;
  alloc_count++;
}

static void alloc_free(uintptr_t ptr)
{
  int i;
  for (i = 0; i < alloc_count; i++) {
    if (allocs[i].ptr == ptr) {
      allocs[i] = allocs[--alloc_count];
      return;
    }
  }
}

static int alloc_find(uintptr_t ptr, size_t *size)
{
  int i;
  for (i = 0; i < alloc_count; i++) {
    if (ptr >= allocs[i].ptr &&
        (uintptr_t)(ptr - allocs[i].ptr) < allocs[i].size) {
      if (size) *size = allocs[i].size - (size_t)(ptr - allocs[i].ptr);
      return 1;
    }
  }
  return 0;
}

/* ----------------------------------------------------------------
 * track expectations
 * ---------------------------------------------------------------- */

typedef struct {
  char expect[256];
  int  must;       /* 1 for =, 0 for ! */
  int  found;
} trk_expect_t;

static trk_expect_t trk_expects[MAX_TRK_EXPECTS];
static int          trk_expect_count;
static int          trk_expect_ignored;

/* forward decls — output helpers defined later */
static void out_fmt(const char *cls, const char *fmt, ...) DBGLOG_PRINTF(2,3);
static void out_line(const char *cls, const char *line);

static char trk_tst_file[512];
static int  trk_tst_line;

static void trk_set_tst(const char *file, int lnum)
{
  if (file && file[0]) {
    snprintf(trk_tst_file, sizeof(trk_tst_file), "%s", file);
    trk_tst_line = lnum;
  }
}

static void trk_reset(void)
{
  trk_expect_count = 0;
  trk_expect_ignored = 0;
  memset(trk_expects, 0, sizeof(trk_expects));
}

static void trk_open(const char *msg)
{
  const char *p = strchr(msg, '[');
  const char *q;

  trk_reset();
  in_trk = 1;
  if (!p) return;
  p++;

  while (*p) {
    int must;

    while (*p == ' ' || *p == '\t' || *p == ',') p++;
    if (*p != '"') break;
    p++;
    if (*p != '=' && *p != '!') break;

    must = (*p == '=');
    p++;

    q = strchr(p, '"');
    if (!q) break;

    if (trk_expect_count < MAX_TRK_EXPECTS) {
      trk_expect_t *e = &trk_expects[trk_expect_count];
      size_t len = (size_t)(q - p) + 1;
      if (len >= sizeof(e->expect)) len = sizeof(e->expect) - 1;
      e->must  = must;
      e->found = 0;
      e->expect[0] = must ? '=' : '!';
      memcpy(e->expect + 1, p, len - 1);
      e->expect[len] = '\0';
      trk_expect_count++;
    } else {
      trk_expect_ignored++;
    }

    p = q + 1;
  }
}

static void trk_scan_line(const char *msg)
{
  int i;
  for (i = 0; i < trk_expect_count; i++) {
    if (!trk_expects[i].found)
      if (strstr(msg, trk_expects[i].expect + 1))
        trk_expects[i].found = 1;
  }
}

static void trk_close(void)
{
  int i;

  if (!in_trk) return;
  in_trk = 0;

  for (i = 0; i < trk_expect_count; i++) {
    trk_expect_t *e = &trk_expects[i];
    int failed = (e->must && !e->found) || (!e->must && e->found);
    const char *cls = failed ? "fail" : "pass";
    if (trk_tst_file[0]) {
      if (failed || !fail_only)
        out_fmt(cls, "%-5s: (%s) %s:%d",
                failed ? "FAIL" : "PASS", e->expect,
                trk_tst_file, trk_tst_line);
    } else {
      if (failed || !fail_only)
        out_fmt(cls, "%-5s: (%s)",
                failed ? "FAIL" : "PASS", e->expect);
    }
  }

  if (trk_expect_ignored) {
    if (trk_tst_file[0])
      out_fmt("fail", "%-5s: (too many dbgtrk expectations; ignored %d after max %d) %s:%d",
              "FAIL", trk_expect_ignored, MAX_TRK_EXPECTS,
              trk_tst_file, trk_tst_line);
    else
      out_fmt("fail", "%-5s: (too many dbgtrk expectations; ignored %d after max %d)",
              "FAIL", trk_expect_ignored, MAX_TRK_EXPECTS);
  }

  trk_reset();
}

/* ----------------------------------------------------------------
 * token helpers
 * ---------------------------------------------------------------- */

static int has_prefix(const char *s, const char *p)
{ return strncmp(s, p, strlen(p)) == 0; }

static uintptr_t parse_ptr_token(const char *s)
{
  return (uintptr_t)strtoull(s, NULL, 16);
}

static size_t parse_size_token(const char *s)
{
  return (size_t)strtoull(s, NULL, 10);
}

static int tokenize(char *msg, char *tok[], int max)
{
  int   nt = 0;
  char *p  = msg;

  while (nt < max) {
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') break;
    tok[nt++] = p;
    while (*p && *p != ' ' && *p != '\t') p++;
    if (*p) { *p = '\0'; p++; }
  }
  return nt;
}

/* ----------------------------------------------------------------
 * M: allocation tracking
 * ---------------------------------------------------------------- */

static void handle_M_colon(char *msg)
{
  char *t[8];
  int   nt;

  if (has_prefix(msg, "M:")) msg += 2;
  nt = tokenize(msg, t, 8);
  if (nt < 1) return;

  if (!strcmp(t[0], "malloc") && nt >= 3)
    alloc_add(parse_ptr_token(t[2]), parse_size_token(t[1]));
  else if (!strcmp(t[0], "free") && nt >= 2)
    alloc_free(parse_ptr_token(t[1]));
  else if (!strcmp(t[0], "calloc") && nt >= 4) {
    size_t count = parse_size_token(t[1]);
    size_t size  = parse_size_token(t[2]);
    if (size && count > ((size_t)-1) / size)
      diag("calloc size overflow in log record");
    else
      alloc_add(parse_ptr_token(t[3]), count * size);
  } else if (!strcmp(t[0], "realloc") && nt >= 4) {
    uintptr_t old = parse_ptr_token(t[1]);
    size_t    sz  = parse_size_token(t[2]);
    uintptr_t nw  = parse_ptr_token(t[3]);
    if (nw) {
      if (old) alloc_free(old);
      alloc_add(nw, sz);
    } else if (old && sz == 0) {
      alloc_free(old);
    }
  } else if (!strcmp(t[0], "strdup") && nt >= 4)
    alloc_add(parse_ptr_token(t[3]), parse_size_token(t[2]));
  else if (!strcmp(t[0], "strndup") && nt >= 4)
    alloc_add(parse_ptr_token(t[3]), parse_size_token(t[2]));
}

/* ----------------------------------------------------------------
 * M? boundary checking
 * ---------------------------------------------------------------- */

static int is_mem_check_op(const char *op)
{
  return !strcmp(op, "strcpy")  || !strcmp(op, "strncpy") ||
         !strcmp(op, "strcat")  || !strcmp(op, "strncat") ||
         !strcmp(op, "memcpy")  || !strcmp(op, "memmove");
}

/* Returns 1 on PASS, 0 on FAIL. Writes result line to *out. */
static int check_M_question(char *msg, char *out, size_t osize)
{
  char     *t[6];
  int       nt;
  uintptr_t p1 = 0;
  size_t    sz = 0, avail = 0;
  int       ok = 1;

  *out = '\0';
  if (has_prefix(msg, "M?")) msg += 2;
  nt = tokenize(msg, t, 6);
  if (nt < 1) {
    snprintf(out, osize, "FAIL: malformed M? record");
    return 0;
  }

  if (!strcmp(t[0], "pointer")) {
    if (nt < 2) {
      snprintf(out, osize, "FAIL: malformed M?pointer record");
      return 0;
    }
    p1 = parse_ptr_token(t[1]);
    ok = (p1 == 0) || alloc_find(p1, NULL);
    snprintf(out, osize, "%-5s: pointer 0x%zX",
             ok ? "PASS" : "FAIL", (size_t)p1);
  } else if (!strcmp(t[0], "memset")) {
    if (nt < 3) {
      snprintf(out, osize, "FAIL: malformed M?memset record");
      return 0;
    }
    p1 = parse_ptr_token(t[1]);
    sz = parse_size_token(t[2]);
    ok = alloc_find(p1, &avail);
    ok = ok && (sz <= avail);
    snprintf(out, osize, "%-5s: memset 0x%zX +%zu (buf=%zu)",
             ok ? "PASS" : "FAIL", (size_t)p1, sz, avail);
  } else if (is_mem_check_op(t[0])) {
    if (nt < 4) {
      snprintf(out, osize, "FAIL: malformed M?%s record", t[0]);
      return 0;
    }
    p1 = parse_ptr_token(t[1]);
    sz = parse_size_token(t[3]);
    ok = alloc_find(p1, &avail);
    if (ok && sz > avail) ok = 0;
    snprintf(out, osize, "%-5s: %s 0x%zX +%zu (buf=%zu)",
             ok ? "PASS" : "FAIL", t[0], (size_t)p1, sz, avail);
  } else {
    snprintf(out, osize, "FAIL: unknown M? operation: %s", t[0]);
    return 0;
  }
  return ok;
}

/* ----------------------------------------------------------------
 * source location extraction
 * ---------------------------------------------------------------- */

static int parse_source(const char *s, char *file, size_t fsize, int *line)
{
  const char *colon = strrchr(s, ':');
  char *end;
  long  lnum;
  size_t flen;

  *line = 0;
  file[0] = '\0';
  if (!colon || colon == s || !isdigit((unsigned char)colon[1])) return 0;

  lnum = strtol(colon + 1, &end, 10);
  if (*end || lnum < 0) return 0;

  flen = (size_t)(colon - s);
  if (flen >= fsize) flen = fsize - 1;
  memcpy(file, s, flen);
  file[flen] = '\0';
  *line = (int)lnum;
  return 1;
}

static void split_line(const char *raw, char *msg, size_t msize,
                       char *file, size_t fsize, int *line)
{
  const char *sep = strrchr(raw, '\x0F');

  *line = 0;
  file[0] = '\0';
  if (sep) {
    size_t mlen = (size_t)(sep - raw);
    if (mlen >= msize) mlen = msize - 1;
    memcpy(msg, raw, mlen);
    msg[mlen] = '\0';
    parse_source(sep + 1, file, fsize, line);
  } else {
    size_t mlen = strlen(raw);
    if (mlen >= msize) mlen = msize - 1;
    memcpy(msg, raw, mlen);
    msg[mlen] = '\0';
  }
}

/* ----------------------------------------------------------------
 * event-code expansion — all markers padded to 6 chars
 * ---------------------------------------------------------------- */

static int event_prefix(const char *msg, const char **word,
                        char *delim, const char **rest)
{
  if (!msg[0] || !msg[1]) return 0;

  if (msg[0] == 'F' && msg[1] == '=') {
    *word = "FAIL";
    *delim = '=';
    *rest = msg + 2;
    return 1;
  }

  *delim = msg[1];
  *rest = msg + 2;

  switch (msg[0]) {
  case 'E': if (*delim == ':') { *word = "ERROR"; return 1; } break;
  case 'W': if (*delim == ':') { *word = "WARN";  return 1; } break;
  case 'I': if (*delim == ':') { *word = "INFO";  return 1; } break;
  case 'P': if (*delim == ':') { *word = "PASS";  return 1; } break;
  case 'F': if (*delim == ':') { *word = "FAIL";  return 1; } break;
  case 'T': if (*delim == '[' || *delim == ']') { *word = "TEST";  return 1; } break;
  case 'K': if (*delim == '[' || *delim == ']') { *word = "TRACK"; return 1; } break;
  case 'V': if (*delim == '[' || *delim == ']') { *word = "VERB";  return 1; } break;
  case 'C': if (*delim == '[' || *delim == ']') { *word = "CLOCK"; return 1; } break;
  case 'M': if (*delim == ':' || *delim == '?') { *word = "MEM";   return 1; } break;
  default: break;
  }
  return 0;
}

/* Rewrite msg prefix: "E:text" → "ERROR:text" (6-char marker).
   Returns 1 if prefix was expanded, 0 if left unchanged.           */
static int expand_prefix(char *msg, size_t msize)
{
  char buf[MAX_LINE];
  const char *word;
  char delim;
  const char *rest;

  if (!event_prefix(msg, &word, &delim, &rest)) return 0;

  /* pad word to 5 chars, then delimiter, then rest */
  {
    size_t pos;
    int n = snprintf(buf, sizeof(buf), "%-5s%c", word, delim);
    if (n < 0) return 0;
    pos = (size_t)n;
    if (pos >= sizeof(buf)) pos = sizeof(buf) - 1;
    if (rest && rest[0] && pos < sizeof(buf) - 1) {
      size_t len;
      buf[pos++] = ' ';
      len = strlen(rest);
      if (len >= sizeof(buf) - pos) len = sizeof(buf) - pos - 1;
      memcpy(buf + pos, rest, len);
      pos += len;
      buf[pos] = '\0';
    }
  }
  snprintf(msg, msize, "%s", buf);
  return 1;
}

/* ----------------------------------------------------------------
 * output helpers — dispatch to stdout or HTML line list
 * ---------------------------------------------------------------- */

typedef struct {
  char  *text;
  char   cls[16];
} out_line_t;

static out_line_t *out_lines;
static int         out_count;
static int         out_cap;

static void html_push(const char *text, const char *cls)
{
  char *copy;

  if (out_count == out_cap) {
    int new_cap = out_cap ? out_cap * 2 : 128;
    out_line_t *new_lines = realloc(out_lines,
                                    (size_t)new_cap * sizeof(*out_lines));
    if (!new_lines) { fputs("dbglog: out of memory\n", stderr); exit(3); }
    out_lines = new_lines;
    out_cap = new_cap;
  }

  copy = strdup(text ? text : "");
  if (!copy) { fputs("dbglog: out of memory\n", stderr); exit(3); }

  out_lines[out_count].text = copy;
  snprintf(out_lines[out_count].cls, sizeof(out_lines[out_count].cls),
           "%s", cls ? cls : "");
  out_count++;
}

static void out_line(const char *cls, const char *line)
{
  if (html_mode)
    html_push(line, cls);
  else
    puts(line);
}

static void out_fmt(const char *cls, const char *fmt, ...) DBGLOG_PRINTF(2,3);

static void out_fmt(const char *cls, const char *fmt, ...)
{
  char buf[MAX_LINE + 1024];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  if (html_mode)
    html_push(buf, cls);
  else
    puts(buf);
}

static void emit_line(const char *msg, const char *file, int lnum, const char *cls)
{
  char expanded[MAX_LINE];
  snprintf(expanded, sizeof(expanded), "%s", msg);
  expand_prefix(expanded, sizeof(expanded));
  if (file[0])
    out_fmt(cls, "%s %s:%d", expanded, file, lnum);
  else
    out_fmt(cls, "%s", expanded);
}

static void emit_fail(const char *msg, const char *file, int lnum)
{
  emit_line(msg, file, lnum, "fail");
}

static void emit_ok(const char *msg, const char *file, int lnum, const char *cls)
{
  if (fail_only) return;
  emit_line(msg, file, lnum, cls);
}

/* ----------------------------------------------------------------
 * line processing — single code path for text and HTML
 * ---------------------------------------------------------------- */

static void process_line(const char *raw)
{
  char msg[MAX_LINE];
  char file[512];
  int  lnum;
  char tmp[MAX_LINE];
  size_t rlen = strlen(raw);

  while (rlen > 0 && (raw[rlen-1] == '\n' || raw[rlen-1] == '\r')) rlen--;
  memcpy(tmp, raw, rlen);
  tmp[rlen] = '\0';

  split_line(tmp, msg, sizeof(msg), file, sizeof(file), &lnum);

  /* --- verbatim close ------------------------------------------- */
  if (strstr(tmp, "V]\x0E") || has_prefix(msg, "V]\x0E")) {
    in_verbatim = 0;
    emit_ok("V]", file, lnum, "verbatim");
    return;
  }

  /* --- inside verbatim block ------------------------------------ */
  if (in_verbatim) {
    if (!fail_only) out_line("verbatim", tmp);
    return;
  }

  /* --- track block ---------------------------------------------- */
  if (in_trk) {
    if (has_prefix(msg, "K]")) {
      trk_close();
      emit_ok("K]", file, lnum, "trk");
      return;
    }
    trk_scan_line(msg);
    return;
  }

  /* --- M: alloc ------------------------------------------------- */
  if (has_prefix(msg, "M:")) {
    char saved[MAX_LINE];
    snprintf(saved, sizeof(saved), "%s", msg);
    handle_M_colon(msg);
    emit_ok(saved, file, lnum, "memory");
    return;
  }

  /* --- M? check ------------------------------------------------- */
  if (has_prefix(msg, "M?")) {
    char saved[MAX_LINE];
    char result[512];
    int  ok;
    snprintf(saved, sizeof(saved), "%s", msg);
    ok = check_M_question(msg, result, sizeof(result));
    if (!fail_only || !ok) {
      emit_line(saved, file, lnum, ok ? "pass" : "fail");
      if (file[0])
        out_fmt(ok ? "pass" : "fail", "%s %s:%d", result, file, lnum);
      else
        out_line(ok ? "pass" : "fail", result);
    }
    return;
  }

  /* --- check pass/fail ----------------------------------------- */
  if (has_prefix(msg, "P:")) { emit_ok(msg, file, lnum, "pass"); return; }
  if (has_prefix(msg, "F:") && !has_prefix(msg, "F="))
    { emit_fail(msg, file, lnum); return; }
  if (has_prefix(msg, "F=")) { emit_fail(msg, file, lnum); return; }

  /* --- track open ---------------------------------------------- */
  if (has_prefix(msg, "K[")) {
    trk_open(msg);
    emit_ok(msg, file, lnum, "trk");
    return;
  }

  /* --- test blocks --------------------------------------------- */
  if (has_prefix(msg, "T["))
    { trk_set_tst(file, lnum); emit_ok(msg, file, lnum, "test"); return; }
  if (has_prefix(msg, "T]")) { emit_ok(msg, file, lnum, "test"); return; }

  /* --- verbatim open ------------------------------------------- */
  if (has_prefix(msg, "V["))
    { in_verbatim = 1; emit_ok(msg, file, lnum, "verbatim"); return; }

  /* --- clock --------------------------------------------------- */
  if (has_prefix(msg, "C[")) { emit_ok(msg, file, lnum, "clock"); return; }
  if (has_prefix(msg, "C]")) { emit_ok(msg, file, lnum, "clock"); return; }

  /* --- diagnostics --------------------------------------------- */
  if (has_prefix(msg, "E:")) { emit_fail(msg, file, lnum); return; }
  if (has_prefix(msg, "W:")) { emit_ok(msg, file, lnum, "warn"); return; }
  if (has_prefix(msg, "I:")) { emit_ok(msg, file, lnum, "info"); return; }

  /* --- fallback ------------------------------------------------ */
  emit_ok(msg, file, lnum, "plain");
}

/* ----------------------------------------------------------------
 * HTML rendering
 * ---------------------------------------------------------------- */

static void html_esc(const char *s)
{
  for (; *s; s++) {
    switch (*s) {
    case '&': fputs("&amp;",  stdout); break;
    case '<': fputs("&lt;",   stdout); break;
    case '>': fputs("&gt;",   stdout); break;
    case '"': fputs("&quot;", stdout); break;
    default:  putchar(*s); break;
    }
  }
}

static void render_html(void)
{
  int i;

  puts("<!doctype html>");
  puts("<html lang=\"en\">\n<head>");
  puts("<meta charset=\"utf-8\">");
  puts("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  puts("<title>dbglog report</title>");
  puts("<style>");
  puts("body{margin:0;font:15px/1.5 system-ui,sans-serif;background:#0f141a;color:#e6edf3}");
  puts("header{padding:1rem 1.25rem;background:#161b22;border-bottom:1px solid #30363d}");
  puts("h1{margin:0;font-size:1.25rem}");
  puts("main{padding:1rem 1.25rem 2rem}");
  puts("pre{margin:0;padding:1rem;overflow:auto;background:#0d1117;font:13px/1.45 ui-monospace,monospace}");
  puts(".line{display:block;white-space:pre}");
  puts(".line.pass{color:#3fb950}.line.fail,.line.error{color:#ff7b72}");
  puts(".line.warn{color:#d29922}.line.info{color:#79c0ff}");
  puts(".line.test{color:#c297ff}.line.clock{color:#a5d6ff}");
  puts(".line.verbatim{color:#8ddb8c}.line.memory{color:#a5d6ff}");
  puts(".line.trk{color:#c297ff}");
  puts("</style>");
  puts("</head>\n<body>");
  puts("<header><h1>dbglog report</h1></header>");
  puts("<main><pre>");

  for (i = 0; i < out_count; i++) {
    printf("<span class=\"line %s\">", out_lines[i].cls);
    html_esc(out_lines[i].text);
    fputs("</span>", stdout);
  }
  putchar('\n');

  puts("</pre></main>\n</body>\n</html>");
}

/* ----------------------------------------------------------------
 * I/O
 * ---------------------------------------------------------------- */

static int process_stream(FILE *fp, const char *name)
{
  char line[MAX_LINE];
  int  ok = 1;
  int  lnum = 0;

  while (fgets(line, sizeof(line), fp)) {
    size_t len;
    lnum++;
    len = strlen(line);
    if (len > 0 && line[len - 1] != '\n' && !feof(fp)) {
      int ch;
      diag("%s:%d: input line exceeds %d bytes", name, lnum, MAX_LINE - 1);
      ok = 0;
      do { ch = fgetc(fp); } while (ch != '\n' && ch != EOF);
      continue;
    }
    process_line(line);
  }

  if (ferror(fp)) {
    fprintf(stderr, "dbglog: error reading %s\n", name);
    return 0;
  }
  return ok;
}

static void usage(FILE *fp, const char *argv0)
{
  fprintf(fp, "usage: %s [-F] [-H] [-h] [-v] [log ...]\n", argv0);
  fprintf(fp, "format dbg.h logs as readable text or HTML\n\n");
  fprintf(fp, "  -F  show only failures\n");
  fprintf(fp, "  -H  render static HTML report\n");
  fprintf(fp, "  -h  show this help and exit\n");
  fprintf(fp, "  -v  show version and exit\n\n");
  fprintf(fp, "  With no log files, dbglog reads from standard input.\n");
}

int main(int argc, char **argv)
{
  int i, ok = 1;

  html_mode = 0;
  fail_only = 0;
  in_verbatim = 0;
  in_trk = 0;

  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (!strcmp(argv[i], "-h")) { usage(stdout, argv[0]); return 0; }
    if (!strcmp(argv[i], "-v")) { puts(DBGLOG_VERSION);     return 0; }
    if (!strcmp(argv[i], "-H")) { html_mode = 1; continue; }
    if (!strcmp(argv[i], "-F")) { fail_only = 1; continue; }
    fprintf(stderr, "dbglog: unknown option %s\n", argv[i]);
    usage(stderr, argv[0]);
    return 2;
  }

  if (i == argc) {
    ok = process_stream(stdin, "stdin");
  } else {
    for (; i < argc; i++) {
      FILE *fp = fopen(argv[i], "r");
      if (!fp) {
        fprintf(stderr, "dbglog: cannot open %s\n", argv[i]);
        ok = 0;
        continue;
      }
      if (!process_stream(fp, argv[i])) ok = 0;
      fclose(fp);
    }
  }

  if (in_trk) {
    diag("unterminated dbgtrk block at end of input");
    ok = 0;
    trk_close();
  }
  if (in_verbatim) {
    diag("unterminated verbatim block at end of input");
    ok = 0;
  }

  if (html_mode) render_html();

  return ok ? 0 : 3;
}
