#define NDEBUG

#include "dbg.h"

#define DBG_NODEBUG_ON DBG_ON
#define DBG_NODEBUG_OFF DBG_OFF

int main(void)
{
  int side = 0;

  dbginf("nodebug dbginf %d", ++side);
  dbgwrn("nodebug dbgwrn %d", ++side);
  dbgerr("nodebug dbgerr %d", ++side);
  dbgchk(++side);
  dbgmst(++side, "nodebug dbgmst");

  dbgtst("nodebug dbgtst block still runs") {
    side += 1;
    dbgchk(++side);
  }

  dbgvrb("nodebug dbgvrb block still runs") {
    side += 2;
  }

  dbgclk("nodebug dbgclk block still runs") {
    side += 4;
  }

  dbgblk {
    side += 8;
  }

  DBG_NODEBUG_ON(side += 16;);
  DBG_NODEBUG_OFF(side += 32;);

  _dbgblk {
    side += 64;
  }

  return side == 7 ? 0 : 1;
}
