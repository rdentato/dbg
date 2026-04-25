#include "dbg.h"


void f_minnie(void) 
{
    dbginf("minnie");
}

int main(int argc, char *argv[])
{
  dbginf("pippo");
  dbgtrk( "1:%>pluto", "0-2:%>minnie") {
    dbginf("topo");
    dbginf("pluto");
    f_minnie();
  }
  dbgtrk("1:top",":plut") {
    dbginf("topo");
    dbginf("pluto");
    f_minnie();
  }
  dbginf("pluto");
}
