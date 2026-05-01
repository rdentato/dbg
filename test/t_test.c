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
  dbgvrb("program stderr sample") {
    fprintf(stderr, "sample program stderr output\n");
  }
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

  x=3;
  dbgtst("(1>x) with x=%d (no message on fail)",x) {
     dbgchk(1>x,"");
  }

  dbgtst("Testing Clock") {
    dbgclk("dbgclk prints the time");
  dbginf("1-----");
  
    x = 100000;
    dbgclk("Testing count to %d",x) {
      for (int k=0; k<x; k++);
    }
  dbginf("2-----");
    x = 100000000;
    dbgclk("Testing count to %d",x) {
      for (int k=0; k< x; k++) ;
    }
      dbginf("3-----");

  }
  
// WILL NOT COMPILE
//  dbgtst("HELLO") {
//    dbgchk(1, "");
//    dbgtst("Hola") {
//      dbgchk(2, "");
//    }
//  }
}
