/*  TEST for dbgnow()
**  Compile with:
**     cc -DDEBUG -Wall -O0 -o t_now t_now.c -I../src
**
**  (C) 2026 by Remo Dentato (rdentato@gmail.com)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dbg.h"

int main(int argc, char *argv[])
{
  dbginf("Testing dbgnow() function (argc: %d)", argc);

  dbgtst("dbgnow with no message") {
    dbgnow();
  }

  dbgtst("dbgnow with a simple message") {
    dbgnow("simple timestamp");
  }

  dbgtst("dbgnow with formatted message") {
    int x = 42;
    dbgnow("x is %d", x);
  }

  dbgtst("two consecutive dbgnow calls") {
    dbgnow("first call");
    dbgnow("second call");
  }

  dbginf("All dbgnow tests completed");
}
