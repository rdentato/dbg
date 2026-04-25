#include <stdio.h>

#include "dbg.h"

#define DBG_ENABLED DBG_ON
#define DBG_DISABLED DBG_OFF

int main(void)
{
  int side = 0;
  int block = 0;
  int ok = 1;

  dbgerr("debug error diagnostic sample");
  dbgwrn("debug warning diagnostic sample");
  dbginf("debug info diagnostic sample");

  dbgvrb("debug verbose block") {
    fprintf(stderr, "program stderr inside dbgvrb\n");
  }

  dbgtst("enabled diagnostics and groups") {
    dbgchk(1);
    dbgchk(side == 0, "disabled calls should not evaluate arguments");
    dbgmst(1, "dbgmst passing scenario");

    DBG_ENABLED(dbgchk(1));
    DBG_DISABLED(dbgchk(++side));

    dbgblk {
      block++;
    }

    _dbgblk {
      block += 100;
    }

    _dbginf("disabled dbginf %d", ++side);
    _dbgwrn("disabled dbgwrn %d", ++side);
    _dbgerr("disabled dbgerr %d", ++side);
    _dbgchk(++side);
    _dbgmst(++side, "disabled dbgmst");

    dbgchk(side == 0, "side=%d", side);
    dbgchk(block == 1, "block=%d", block);
  }

  dbgclk("enabled clock block") {
    block += 0;
  }

  if (side != 0 || block != 1) ok = 0;
  return ok ? 0 : 1;
}
