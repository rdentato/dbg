#define DEBUG_ALLOC

#include "dbg.h"

int main(void)
{
  int ok = 1;

  dbgtst("allocation and memory wrappers") {
    char *p = malloc(32);
    char *q = calloc(16, 1);
    char *s;
    char *t;
    char *r;

    dbgchk(p != NULL, "malloc failed");
    dbgchk(q != NULL, "calloc failed");

    if (p == NULL || q == NULL) return 1;

    dbgptr(p);
    strcpy(p, "ab");
    dbgchk(strcmp(p, "ab") == 0, "p=%s", p);
    if (strcmp(p, "ab")) ok = 0;

    strcat(p, "cd");
    dbgchk(strcmp(p, "abcd") == 0, "p=%s", p);
    if (strcmp(p, "abcd")) ok = 0;

    strncat(p, "ef", 2);
    dbgchk(strcmp(p, "abcdef") == 0, "p=%s", p);
    if (strcmp(p, "abcdef")) ok = 0;

    strncpy(q, "xy", 3);
    dbgchk(strcmp(q, "xy") == 0, "q=%s", q);
    if (strcmp(q, "xy")) ok = 0;

    memcpy(q, p, 3);
    q[3] = '\0';
    dbgchk(strcmp(q, "abc") == 0, "q=%s", q);
    if (strcmp(q, "abc")) ok = 0;

    memmove(q + 1, q, 3);
    q[4] = '\0';
    dbgchk(strcmp(q, "aabc") == 0, "q=%s", q);
    if (strcmp(q, "aabc")) ok = 0;

    memset(q, 'z', 2);
    dbgchk(q[0] == 'z' && q[1] == 'z', "q[0]=%c q[1]=%c", q[0], q[1]);
    if (q[0] != 'z' || q[1] != 'z') ok = 0;

    r = realloc(p, 64);
    dbgchk(r != NULL, "realloc failed");
    if (r != NULL) p = r;

    s = strdup("hello");
    t = strndup("abcdef", 3);
    dbgchk(s != NULL && strcmp(s, "hello") == 0, "strdup result invalid");
    dbgchk(t != NULL && strcmp(t, "abc") == 0, "strndup result invalid");
    if (s == NULL || strcmp(s, "hello")) ok = 0;
    if (t == NULL || strcmp(t, "abc")) ok = 0;

    free(p);
    free(q);
    free(s);
    free(t);
  }

  return ok ? 0 : 1;
}
