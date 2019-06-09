# dbg
Very very minimal debug/testing macro.

## API

```
  dbgmsg(char *, ...)        --> Output a message on stderr (works as printf(...)).
                                 If DEBUG is not defined, do nothing.

  dbgchk(test, char *, ...)  --> Perform the test. If test fails prints a message on
                                 stderr (works as printf(...)).
                                 If DEBUG is not defined, do nothing.

  _dbgmsg(char *, ...)       --> Do nothing. Used to disable the debug message.

  _dbgchk(test, char *, ...) --> Do nothing. Used to disable the debug message.

```
  Note that NDEBUG has precedence over DEBUG

## Statistics

  To summarize the passed/failed tests you can use the `dbgstat` utility that can
be compiled with:

```
  cp dbg.h dbgstat.c; cc -DDBGSTAT -Wall -o dbgstat dbgstat.c
```
and use it as a filter. For example:
```
  myprg 2> my.log
  dbgstat < my.log
```

will output (assuming all 12 tests you defined with `dbgcheck(...)` passed):
```
  PASS: 12
  FAIL: 0
```
and will return `0` to the shell.  

If there is any failure, `dbgstat` will exit with an error code. This way you can signal you build/integration pipeline that something went wrong.

## Self test

  To see `dbg`in action, complie the self test with:
  
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
int main(int argc, char *argv[])
{
  int x;
  dbgmsg("Test %s (argc: %d)","message 1",argc);

  x=0;
  dbgmsg("Testing (1>x) with x=%d",x);
  dbgchk(1>x,"x=%d\n",x); // WILL PASS!

  x=1;
  dbgmsg("Testing (1>x) with x=%d",x); 
  dbgchk(1>x,"x=%d\n",x);  // WILL FAIL!

  x=2;
  dbgmsg("Testing (1>x) with x=%d",x);
  dbgchk(1>x,"x=%d\n",x); // WILL FAIL!

  x=2;
  dbgmsg("Testing (1>x) with x=%d(no message on fail)",x);
  dbgchk(1>x,"");        // WILL FAIL!
}
```


  
