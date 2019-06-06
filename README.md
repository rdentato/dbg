# dbg
Very very minimal debug/testing macro.


  dbgmsg(char *, ...)        --> Output a message on stderr (works as printf(...)).
                                 If DEBUG is not defined, do nothing.

  dbgchk(test, char *, ...)  --> Perform the test. If test fails and format string is not
                                 NULL, prints a message on stderr (works as printf(...)).
                                 If DEBUG is not defined, do nothing.

  _dbgmsg(char *, ...)       --> Do nothing. Used to disable a debug message.

  _dbgchk(test, char *, ...) --> Do nothing. Used to disable a debug message.

  Note that NDEBUG has precedence over DEBUG
