#define DEBUG_ALLOC

#include "dbg.h"

int main(void)
{
  dbgtst("allocation boundary violations") {

    /* --- use-after-free --------------------------------------- */
    char *p = malloc(16);
    dbgchk(p != NULL, "malloc failed");
    strcpy(p, "safe");
    free(p);
    dbgptr(p);                            /* M? FAIL: freed */

    /* --- out-of-bounds write --------------------------------- */
    char *q = malloc(8);
    dbgchk(q != NULL, "malloc failed");
    strcpy(q, "overflow!");               /* M? FAIL: 10 bytes into 8 */

    /* --- realloc, use old pointer ----------------------------- */
    char *r = malloc(32);
    dbgchk(r != NULL, "malloc failed");
    char *r2 = realloc(r, 64);
    dbgchk(r2 != NULL, "realloc failed");
    r = r2;
    dbgptr(r);                             /* M? PASS: new pointer valid */
    /* --- memset past allocation ------------------------------- */
    char *s = malloc(12);
    dbgchk(s != NULL, "malloc failed");
    memset(s, 0, 20);                     /* M? FAIL: 20 > 12 */

    /* --- valid writes for contrast ---------------------------- */
    char *t = malloc(32);
    dbgchk(t != NULL, "malloc failed");
    strcpy(t, "ok");
    dbgptr(t);                             /* M? PASS */
    memset(t, 'A', 16);
    dbgptr(t);                             /* M? PASS */
    dbgptr(t + 16);                        /* M? PASS: within bounds */

    /* --- pointer just past end -------------------------------- */
    dbgptr(t + 32);                        /* M? FAIL: past allocation */
    dbgptr(t + 40);                        /* M? FAIL: way past */

    /* --- null pointer check ----------------------------------- */
    dbgptr(0);                             /* M? PASS: null always ok */

    /* --- free then check -------------------------------------- */
    free(t);
    dbgptr(t);                             /* M? FAIL: freed */

    /* --- non-heap / unallocated address ----------------------- */
    dbgptr("xca");                         /* M? FAIL: not from malloc */
    dbgptr((void *)0xDEAD);                 /* M? FAIL: bogus address */

    /* --- strcat overflow -------------------------------------- */
    char *u = malloc(4);
    dbgchk(u != NULL, "malloc failed");
    u[0] = '\0';
    strcat(u, "toolong");                 /* M? FAIL: needs 8 bytes */

    free(q);
    free(s);
    free(u);
  }

  return 0;
}
