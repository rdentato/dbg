# dbg
Very very minimal debug/testing macro.

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
