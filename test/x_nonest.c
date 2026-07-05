#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbg.h"

int main(void)
{

  dbgtst("(1>0)") {
    dbgchk(1>0,"??");
  }

  dbgtst("(0>1)") {
    dbgtst("(0>1)") {
      dbgchk(0>1,"??");
    }
  }
}
