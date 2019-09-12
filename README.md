# dbg [![Build Status](https://travis-ci.org/rdentato/dbg.svg?branch=master)](https://travis-ci.org/rdentato/dbg)  

Very very minimal debug/testing macro.

## Using dbg.h

Just include `dbg.h` in your code and define `DEBUG` to the desired level (see below) before compiling.

## API

```
   DEBUG                     --> If undefined, removes all debug instructions.
                                 If defined sets the level of debugging and enables
                                 the dbg functions:

                                          level         enabled functions
                                      ------------  --------------------------
                                      DBGLVL_ERROR  dbgerr() dbgmsg() dbgprt()
                                      DBGLVL_WARN   as above plus dbgwrn()
                                      DBGLVL_INFO   as above plus dbginf()
                                      DBGLVL_TEST   all dbg functions.

                                 Note NDEBUG has higher priority than DEBUG, if
                                 NDEBUG is defined, DEBUG will be undefined.

   dbgmsg(char *, ...)       --> Prints a message on stderr (works as printf(...)).
                                 If DEBUG is not defined, do nothing.
 
   dbgprt(char *, ...)       --> Prints a message on stderr (works as printf(...)) omitting
                                 filename and line. If DEBUG is not defined, do nothing.
 
   dbginf(char *, ...)       --> Prints an "INFO:" message depending on the DEBUG level.
   dbgwrn(char *, ...)       --> Prints a  "WARN:" message depending on the DEBUG level.
   dbgerr(char *, ...)       --> Prints an "FAIL:" message.
   dbgtst(char *, ...)       --> Starts a test case.
                                 If DEBUG is undefined or lower than DBGLVL_TEST, do nothing.

   dbgchk(test, char *, ...) --> Perform the test and set errno (0: ok, 1: ko). If test fails
                                 prints a message on stderr (works as printf(...)).
                                 If DEBUG is undefined or lower than DBGLVL_TEST, do nothing.
 
   dbgreq(test, char *, ...) --> As dbgchk() but if test fails exit the program with abort().
 
   dbgclk {...}              --> Measure the time needed to execute the block. If DEBUG is
                                 undefined or lower than DBGLVL_TEST, execute the block but
                                 don't measure the elapsed time.
 
   dbgblk {...}              --> Execute the block only if DEBUG is defined as DBGLVEL_TEST.

   dbgtrk( ... ) {...}       --> Specify the strings to be tracked within the scope of the
                                 block. If DEBUG is not defined or lower than DBGLVL_TEST,
                                 execute the block but don't mark track strings.
                                 (See 'dbgstat' in the tools directory for documentation)

  _dbgmsg(char *, ...)       --> Do nothing. Used to disable the debug message.
  _dbgprt(char *, ...)       --> Do nothing. Used to disable the debug message.
  _dbginf(char *, ...)       --> Do nothing. Used to disable the debug message.
  _dbgwrn(char *, ...)       --> Do nothing. Used to disable the debug message.
  _dbgerr(char *, ...)       --> Do nothing. Used to disable the debug message.
  _dbgchk(test, char *, ...) --> Do nothing. Used to disable the debug message.
  _dbgreq(test, char *, ...) --> Do nothing. Used to disable the debug message.
  _dbgclk {...}              --> Execute the block but don't measure time.
  _dbgblk {...}              --> Do not execute the code block.
  _dbgtrk( ... ) {...}       --> Execute the block but don't mark string tracking.

```

## Statistics

  To summarize the passed/failed tests you can use the `dbgstat` utility in the tools
directory.

For example, assume you have 12 tests :
```
  myprg 2> my.log
  dbgstat -q < my.log
```

will output (assuming all 12 tests you defined with `dbgcheck(...)` passed):
```
  PASS: 12
  FAIL: 0
```
and will return `0` to the shell.  

If there is any failure, `dbgstat` will exit with an error code. This way you can signal your build/integration pipeline that something went wrong.

## Self test

  To see `dbg`in action, compile the self test with:
  
```
  cp dbg.h dbgtest.c
  cc -DDEBUG -DDBGTEST -Wall -o dbgtest dbgtest.c
```
and run it with:
```
  dbgtest 2>&1 | dbgstat
```

You should get an output similar to:
```
  PASS: 1
  FAIL: 3
```

The failures are there just to show what is supposed to happen!
Look at the source code of dbgtest.c to see what's going on:

``` C
#include "dbg.h"

int main(int argc, char *argv[])
{
  int x;

  x=0;
  dbgtest("Testing (1>x) with x=%d",x);
  dbgchk(1>x,"x=%d\n",x); // WILL PASS!

  x=1;
  dbgtest("Testing (1>x) with x=%d",x); 
  dbgchk(1>x,"x=%d\n",x);  // WILL FAIL!

  x=2;
  dbgtest("Testing (1>x) with x=%d",x);
  dbgchk(1>x,"x=%d\n",x); // WILL FAIL!

  x=2;
  dbgtest("Testing (1>x) with x=%d(no message on fail)",x);
  dbgchk(1>x,"");        // WILL FAIL!
}

```


  
