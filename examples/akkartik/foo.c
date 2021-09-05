/*
** See:  http://akkartik.name/post/tracing-tests
**
** Quoting Kartik from the page above:
**   "I've been experimenting with a new approach [...]. 
**    Rather than pass in specific inputs and make
**    assertions on the outputs, I modify code to
**    judiciously print to a trace and make assertions
**    on the trace at the end of a run."
**
** dbg implements this approach via the dbgtrk() instruction:
**
**    dbgtrk(pat1, pat2, pat3, ...) {
**      ... scope ...  
**    }
** 
**   The "assertions" Kartik talks about are expressed as 
** "patterns" (pat1, pat2, ...) that must (or must not) match
** the lines in the trace emitted by the code in the scope.
**
**   Actually, what "dbgtrk() {...}" does is just to mark
** the portion of the trace we want to examine. The actual
** tracking is done by the 'dbgstat' program in the 'tools'
** directory.
**  
**  To see it in action, compile everything with:
**
**   examples/akkartik> ../../bld cleanall
**   examples/akkartik> ../../bld foo
**   examples/akkartik> ./foo 2>&1 | ../../tools/dbgstat.exe
**
**  or just have a look at the foo.log file.
*/

#include <stdlib.h>
#include "dbg.h"

int fooA(int x) {
  dbginf("foo: %d",x); // That's the string we want to track
  return x;
}

int fooB(int x) {
  dbginf("foo:%d",x); // We forgot to add a space in this function!!
  return x;
}

int bar(int x) {
  dbginf("bar: %d",x);
  return x;
}

void test_foo() {
           // expect 1 or mores string matching that pattern
          //              // expect no string matching this pattern
  dbgtrk("=foo:$S$d" "$," "!foo:$S0") {
    fooA(12); // This will PASS  
    fooB(43); // This will PASS
  }
  dbgtrk("!foo:$0") { // Here we only care for not having 0 as an argument
    fooA(0);  // This will FAIL
    fooB(43); // This will be ignored
  }
  dbgtrk("!foo:$w$D$a" "$,"  "!sd" "$,"  "sllll") { // This will FAIL
    bar(21);  
  }
}

int main (int argc, char *argv[])
{
  test_foo();
  exit(0);
}
 