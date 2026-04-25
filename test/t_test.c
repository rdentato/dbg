/*  ************ TEST SUITE *************
**  Compile with:
**     cc -DDEBUG -Wall -O0 -o dbgtest dbgtest.c
*/

/* 
**  (C) 2018 by Remo Dentato (rdentato@gmail.com)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dbg.h"

int main(int argc, char *argv[])
{
  int volatile x;

  dbginf("Testing %s (argc: %d)","dbg library",argc);
  dbgchk(1,"");

  x=0;
  dbgtst("(1>x) with x=%d",x) {
    dbgchk(1>x,"x=%d",x);
  }

  x=1;
  dbgtst("(1>x) with x=%d",x) {
    dbgchk(1>x,"x=%d",x);
  }

  x=2;
  dbgtst("(1>x) with x=%d",x) {
    dbgchk(1>x,"x=%d",x);
  }

  dbgblk {
    int e = errno, x=0;
    if (e) {
      dbginf("Sigh it failed (%d)",e-x);
    }
  }

  x=3;
  dbgtst("(1>x) with x=%d (no message on fail)",x) {
     dbgchk(1>x,"");
  }

  _dbgblk {
    int e = errno;
    if (e) {
      dbginf("Sigh it failed: (%d) but I'll never know (%d)",e,x);
    }
  }

  dbgtst("Testing Clock") {
    dbgclk("dbgclk prints the time");
  dbgmsg("1-----");
  
    x = 100000;
    dbgclk("Testing count to %d",x) {
      for (int k=0; k<x; k++);
    }
  dbgmsg("2-----");
    x = 100000000;
    dbgclk("Testing count to %d",x) {
      for (int k=0; k< x; k++) ;
    }
      dbgmsg("3-----");

  }
}


