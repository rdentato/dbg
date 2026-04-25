#undef DEBUG
#define DEBUG 1

#include <stdio.h>

#include "dbg.h"

int main(void)
{
  int side = 0;

  dbgerr("warn-level error diagnostic");
  dbgwrn("warn-level warning diagnostic");
  dbginf("warn-level info should be disabled %d", ++side);
  dbgchk(++side, "");

  dbgvrb("warn-level verbose block") {
    side += 1;
    fprintf(stderr, "warn-level program stderr\n");
  }

  dbgtst("test marker disabled below DBGLVL_TEST") {
    side += 10;
    dbgchk(++side, "");
  }

  dbgclk("clock marker disabled below DBGLVL_TEST") {
    side += 2;
  }

  dbgblk {
    side += 100;
  }

  return side == 13 ? 0 : 1;
}
