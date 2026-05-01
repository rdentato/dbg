/*  TEST for dbgtrk()
**  Compile with:
**     cc -DDEBUG -Wall -O0 -o t_trk t_trk.c -I../src
**
**  (C) 2026 by Remo Dentato (rdentato@gmail.com)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dbg.h"

int main(int argc, char *argv[])
{
  int volatile x;

  dbginf("Testing dbgtrk() function (argc: %d)", argc);

  dbgtst("dbgtrk with a single expected string") {
    dbgtrk("=expected-string") {
      dbginf("expected-string");
    }
  }

  dbgtst("dbgtrk with must-exist and must-not-exist strings") {
    dbgtrk("=must-be-present", "!must-be-absent") {
      dbginf("must-be-present");
      /* must-be-absent is NOT printed */
    }
  }

  dbgtst("dbgtrk with empty block") {
    dbgtrk("=whatever") {
      /* intentionally empty */
    }
  }

  x = 42;
  dbgtst("dbgtrk block that uses local variables, x=%d", x) {
    dbgtrk("=variable-42") {
      dbginf("variable-%d", x);
    }
  }

  dbgtst("dbgtrk inside dbgblk") {
    dbgblk {
      dbgtrk("=inside-dbgblk") {
        dbginf("inside-dbgblk");
      }
    }
  }

  dbgtst("dbgtrk with multiple must-not-exist strings") {
    dbgtrk("!absent-one", "!absent-two", "!absent-three") {
      dbginf("present-one");
    }
  }

  dbgtst("dbgtrk with a must-not-exist string that DOES appear") {
    dbgtrk("=present", "!present") {
      dbginf("present");
    }
  }

  dbgtst("dbgtrk with no expectations at all") {
    dbgtrk() {
      dbginf("anything");
    }
  }

  dbgtst("two TRK blocks in sequence (no state leakage)") {
    dbgtrk("=first") {
      dbginf("first");
    }
    dbgtrk("=second") {
      dbginf("second");
    }
  }

  dbgtst("dbgtrk with substring matching") {
    dbgtrk("=ab", "!xyz") {
      dbginf("abc");  /* "ab" is substring of "abc" */
      /* "xyz" is NOT a substring of any line */
    }
  }

  dbgtst("dbgtrk with 8 expectations (max)") {
    dbgtrk("=s0", "=s1", "=s2", "=s3", "=s4", "=s5", "=s6", "=s7") {
      dbginf("s0 s1 s2 s3 s4 s5 s6 s7");
    }
  }

  dbgtst("dbgtrk with 9 expectations (overflow)") {
    dbgtrk("=e0", "=e1", "=e2", "=e3", "=e4", "=e5", "=e6", "=e7", "=e8") {
      dbginf("e0");
    }
  }

  dbgtst("dbgtrk with buffer overflow (strings too long)") {
    dbgtrk("=This string is intentionally very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long to overflow the buffer and test that path") {
      /* nothing */
    }
  }

  dbginf("All dbgtrk tests completed");
}
