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

  dbginf("All dbgtrk tests completed");
}
