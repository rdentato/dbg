#include "dbg.h"


void f_minnie(void) 
{
    dbgmsg("minnie");
}

int main(int argc, char *argv[])
{
  dbgmsg("pippo");
  dbgtrk( "1:%>pluto", "0-2:%>minnie") {
    dbgmsg("topo");
    dbgmsg("pluto");
    f_minnie();
  }
  dbgtrk("1:top",":plut") {
    dbgmsg("topo");
    dbgmsg("pluto");
    f_minnie();
  }
  dbgmsg("pluto");
}
