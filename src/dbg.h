/* 
**  (C) 2026 by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

//  # DEBUG AND TESTING MACROS

#ifndef DBG_VERSION
#define DBG_VERSION     0x0201000
#define DBG_VERSION_STR "dbg 2.1.0"

//  ## Usage
//  To enable the debugging functions, DEBUG must be defined before 
//  including dbg.h. See section "## Debugging levels" for more details.
//  To enable memory allocation debugging, define DEBUG_ALLOC.
//
//  Note that NDEBUG has higher priority than DEBUG and DEBUG_ALLOC:
//  if NDEBUG is defined, then DEBUG and DEBUG_ALLOC will be undefined.

#ifdef NDEBUG
#ifdef DEBUG
#undef DEBUG
#endif
#ifdef DEBUG_ALLOC
#undef DEBUG_ALLOC
#endif
#endif

// Set it to DBGLVL_ERROR
#if defined(DEBUG_ALLOC) && !defined(DEBUG)
#define DEBUG 0
#endif

// ## Disabling functions
// For each function provided by `dbg.h`, there is a *temporarily disabled*
// counterpart whose name begins with one underscore.
// This avoids having to comment out or delete debugging functions that
// are not needed at the moment but can be useful at a later time.
// Note that from a C standard point of view, this is a compliant usage
// as all these symbols have file scope. (7.1.3)
// The meaning and usage of `DBG_OFF()` is detailed in the section
// titled "Grouping".

#define DBG_OFF(...)

#define _dbgtst DBG_OFF
#define _dbgtrk DBG_OFF
#define _dbginf DBG_OFF
#define _dbgvrb DBG_OFF
#define _dbgwrn DBG_OFF
#define _dbgerr DBG_OFF
#define _dbgchk DBG_OFF
#define _dbgmst DBG_OFF
#define _dbgptr DBG_OFF
#define _dbgclk DBG_OFF
#define _dbgnow DBG_OFF
#define _dbgblk while(0)

// ## No DEBUG
// If DEBUG is not defined, the dbgxxx symbol should atill be defined,
// to ensure the code still compiles, but they should do nothing.

#define DBG_ON  DBG_OFF
#define dbgtst _dbgtst
#define dbgtrk _dbgtrk
#define dbginf _dbginf
#define dbgvrb _dbgvrb
#define dbgwrn _dbgwrn
#define dbgerr _dbgerr
#define dbgchk _dbgchk
#define dbgmst _dbgmst
#define dbgclk _dbgclk
#define dbgnow _dbgnow
#define dbgblk _dbgblk
#define dbgptr _dbgptr

#ifdef DEBUG

// Ask for strdup()/strndup() (but we can't be sure!)
#define __STDC_WANT_LIB_EXT2__ 1

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// This variable will always be 0. It's used to suppress warnings
// and ensures that some debugging code is not optimized.
static volatile int dbg_zero = 0;

// ## Debugging Level
//
//   DEBUG       If undefined, removes all debug instructions.
//               If defined sets the level of debugging and enables
//               the dbg functions:
//
//                     level         enabled functions
//                 ------------  --------------------------
//                 DBGLVL_ERROR  dbgerr() dbgvrb()
//                 DBGLVL_WARN   as above plus dbgwrn()
//                 DBGLVL_INFO   as above plus dbginf()
//                 DBGLVL_TEST   dbg functions except dgptr()
//
//   DEBUG_ALLOC  Enables redefinition of malloc(), free(), realloc(),
//                calloc(), strdup() and strndup(). Implies DEBUG if
//                DEBUG is not already defined.

#define DBGLVL_ERROR 0
#define DBGLVL_WARN  1
#define DBGLVL_INFO  2
#define DBGLVL_TEST  3

// ## Printing messages
//
//   dbgvrb(char *, ...) {...} Marks enclosed stderr output as program output.
//   dbginf(char *, ...)      Prints an "INFO:" message depending on the DEBUG level.
//   dbgwrn(char *, ...)      Prints a  "WARN:" message depending on the DEBUG level.
//   dbgerr(char *, ...)      Prints a  "FAIL:" message.

// Internal helpers
#define dbg_msg(...)  (fprintf(stderr,__VA_ARGS__),     \
                      fprintf(stderr," \x0F%s:%d\n",__FILE__,__LINE__), \
                      dbg_zero)
#undef  dbgerr
#define dbgerr(...)   dbg_msg("EROR: " __VA_ARGS__)

#undef  dbgvrb
#define dbgvrb(...)   for (int dbg_ = 1; \
                        dbg_ && !dbg_msg("VRB[: " __VA_ARGS__); \
                        dbg_ = (fputs("VRB]:\n",stderr) , 0))

// Use dbgvrb() around code that intentionally writes to stderr so later
// log processing can distinguish program output from dbg-generated diagnostics.

#if DEBUG >= DBGLVL_WARN
#undef  dbgwrn
#define dbgwrn(...)   dbg_msg("WARN: " __VA_ARGS__)
#endif

#if DEBUG >= DBGLVL_INFO
#undef  dbginf
#define dbginf(...)   dbg_msg("INFO: " __VA_ARGS__)
#endif

// ## Testing 
//   dbgtst(char *, ...){ ...}     Starts a test case.
//                                 If DEBUG is undefined or lower than DBGLVL_TEST, do nothing.
//                                 dbgtst blocks cannot be nested.
//                                 Statistics can be collected separately by external log tools.
//
//   dbgchk(test, char *, ...)     Perform the test and set errno (0: ok, 1: ko).
//                                 The second argument is mandatory: pass "" when
//                                 no failure message is needed.
//                                 If the message is not empty and the test fails,
//                                 it is printed on stderr (works as printf(...)).
//                                 If DEBUG is undefined or lower than DBGLVL_TEST, do nothing.
// 
//   dbgmst(test, char *, ...)     As dbgchk() but if test fails exit the program with abort().
// 
//   dbgblk {...}                  Execute the block only if DEBUG is defined as DBGLVEL_TEST.
//

#if DEBUG >= DBGLVL_TEST

static char *dbg_nested_test = "";

#undef  dbgtst
#define dbgtst(...) \
     if (dbg_nested_test[0]) ; \
     else for (int dbg_nested_test = 1; \
               dbg_nested_test && !dbg_msg("TST[: " __VA_ARGS__); \
               fputs("TST]:\n",stderr), dbg_nested_test = 0)

#undef  dbgchk

#define dbg_fst(x,...) x
#define dbgchk(e,...) \
  do { \
    int dbg_err=!(e); \
    fprintf(stderr,"%s: (%s) \x0F%s:%d\n",(dbg_err?"FAIL":"PASS"),#e,__FILE__,__LINE__); \
    char *dbg_errmsg = "" dbg_fst(__VA_ARGS__,0);\
    if (dbg_err && (dbg_errmsg[0] != '\0')) { fprintf(stderr,"    ` " __VA_ARGS__); fputc('\n',stderr); } \
    errno = dbg_err; \
  } while(0)

#undef  dbgmst
#define dbgmst(e,...)  do { dbgchk(e,__VA_ARGS__); if (errno) abort();} while(0)

// ## Tracing
//
// Kartik Agaram exposed a nice idea about using traces to write better
// tests: https://akkartik.name/post/tracing-tests
// dbgtrk() is a simple implementation of that idea. It allows you to
// specify which strings you expect to be present (or to be missing) in
// a give portion of log
//
// dbgtrk("=must-exist","!must-not-exist") {
//   code_emitting_log
// }
//
// The actual test will be performed by dbglog that will check the strings
// and inject a "PASS" or "FAIL".
//
// dbgtrk can't be nested.

static char *dbg_nested_track = "";
#undef  dbgtrk
#define dbgtrk(...) \
     if (dbg_nested_track[0]) ; \
     else for (int dbg_nested_track = 1; \
               dbg_nested_track && !dbg_msg("TRK[: %s", #__VA_ARGS__); \
               fputs("TRK]:\n",stderr), dbg_nested_track = 0)


#undef  dbgblk
#define dbgblk     if (dbg_zero) ; else

// ## Profiling
//  The `dbgclk` function is intended as a quick and dirty way to determine the elapsed time
//  spent in a block of code. It's accuracy depends on the implementation of the `clock()`
//  function. The elapsed time is reported as a fraction:
//     Examples:
//        CLK]: +64000/1000000 sec. t_test.c:67   -> 64 milliseconds
//        CLK]: +64/1000 sec. t_test.c:67         -> 64 milliseconds
//
//   dbgclk(char *, ...) {...}     Measure the time needed to execute the block. If DEBUG is
//                                 undefined or lower than DBGLVL_TEST, execute the block but
//                                 don't measure the elapsed time.
// 
//  _dbgclk(char *, ...) {...}     Execute the block but don't measure time.
//  

typedef struct {
  clock_t    clk;       time_t     time;    
  char       tstr[32];  struct tm *time_tm; 
  long int   elapsed; 
} dbgclk_t;

#undef  dbgclk
#define dbgclk(...)  \
  for (dbgclk_t dbg_ = {.elapsed = -1};   \
      \
       (dbg_.elapsed < 0) && ( \
          time(&dbg_.time), dbg_.time_tm=localtime(&dbg_.time),    \
          strftime(dbg_.tstr,32,"%Y-%m-%d %H:%M:%S",dbg_.time_tm),\
          fprintf(stderr,"CLK[: %s",dbg_.tstr), dbg_msg(" " __VA_ARGS__) , \
          dbg_.clk = clock() \
       ) ;   \
      \
       dbg_.elapsed=(long int)(clock()-dbg_.clk),               \
       fprintf(stderr,"CLK]: +%ld/%ld sec.\n", dbg_.elapsed, (long int)CLOCKS_PER_SEC) \
     )

#undef dbgnow
#define dbgnow(...)  \
  for (dbgclk_t dbg_ = {.elapsed = -1}   \
      ; \
       (dbg_.elapsed < 0) && ( \
          time(&dbg_.time), dbg_.time_tm=localtime(&dbg_.time),    \
          strftime(dbg_.tstr,32,"%Y-%m-%d %H:%M:%S",dbg_.time_tm),\
          fprintf(stderr,"NOW=: %s",dbg_.tstr), dbg_msg(" " __VA_ARGS__) \
       )\
      ; \
       dbg_.elapsed=1 \
     )

// ## Grouping
// 
//  Say you are testing/developing a function and you placed some debugging 
//  code related to that function in various places in your code.
//  Now that everything works fine, you have to go back and delete or
//  comment out them to avoid having your log cluttered with now useless
//  messages. A little bit inconvenient, expecially if you feel 
//  you may have to re-enable them at a later stage at a later time.
//  In case like this, you can plan in advance and define a "debugging 
//  group". An example may clarify the concept better:
// 
//      /* Group to debug/trace ingestion phase */
//      #define DBG_CHECK_INGEST DBG_ON  
//      ...
//      DBG_CHECK_INGEST(dbgchk(dataread > 0,"No data read! (%d)",dataread));
//      ... (some functions later)
//      DBG_CHECK_INGEST(dbginf("Read %d read so far", dataread));
//      ... (into a function further away)
//      DBG_CHECK_INGEST(dbgchk(feof(f),"File had still data to read!"));
//
//  Defining `DBG_CHECK_INGEST` as `DBG_ON` the code will be compiled in,
//  defining it as `DBG_OFF` will compile them out.
//
//   DBG_ON      Turn a debugging group ON
//   DBG_OFF     Turn a debugging group OFF
//

#undef  DBG_ON
#define DBG_ON(...)  __VA_ARGS__

static inline int dbg_avoid_warning() { return (dbg_nested_test[0] && dbg_nested_track[0]) ;}

#endif // DEBUG >= DBGLVL_TEST

#ifdef DEBUG_ALLOC

// ## Memory usage
//  
//  If DEBUG_ALLOC is defined, each call to the memory
//  related functions will produce a line in the log like this:
//
//       MTRK: function(args) ptr_in size ptr_out
//
//  0 N P malloc, calloc, strdup, realloc(NULL,N)
//  P 0 0 free, realloc(P,0)
//  P N P realloc(P,N)
//
//  The function dbgptr is meant to verify that a pointer to allocated
//  memory is "valid", i.e. that it belongs to a block previously allocated:
// 
//    dbgptr(void *p)   Checks if the pointer p is witin a block
//                      allocated with malloc(), calloc(), etc.
//
//  It can be helpful to check for buffer overruns
//

#include <stdlib.h>
#include <inttypes.h>

#define dbg_write(...)    (fprintf(stderr,__VA_ARGS__))
#define dbg_writeln(...)  (fprintf(stderr,__VA_ARGS__) , fprintf(stderr," \x0F%s:%d\n",file,line))

// The only reason we need dbg_p is that %p representation of NULL pointers is
// compiler dependant. This way the pointer is uniquely converted in an integer.
// We use this as a stable log label and never convert it back to a pointer.
#define dbg_p(x)  ((uintptr_t)(x))

static inline void *dbg_malloc(size_t size, const char *file, int line)
{
  dbg_write("MTRK: malloc(%zd) 0 %zd ",size,size);
  void *ptr = malloc(size);
  dbg_writeln("%zX",dbg_p(ptr));
  return ptr;
}

static inline void dbg_free(void *ptr, const char *file, int line)
{
  dbg_writeln("MTRK: free(%zX) %zX 0 0",dbg_p(ptr),dbg_p(ptr));
  free(ptr);
}

static inline void *dbg_calloc(size_t count, size_t size, const char *file, int32_t line)
{
  dbg_write("MTRK: calloc(%zd,%zd) 0 %zd ",count,size,count*size);
  void *ptr = calloc(count,size);
  dbg_writeln("%zX",dbg_p(ptr));
  return ptr;
}

static inline void *dbg_realloc(void *ptr, size_t size, const char *file, int32_t line)
{
  dbg_write("MTRK: realloc(%zX,%zd) %zX %zd ",dbg_p(ptr),size,dbg_p(ptr),size);
  ptr = realloc(ptr,size);
  dbg_writeln("%zX",dbg_p(ptr));
  return ptr;
}

// strdup() is somewhat special being a Posix but a C standard function
char *strdup(const char *s);
char *strndup(const char *s, size_t size);

static inline char *dbg_strdup(const char *s, const char *file, int line)
{
  dbg_write("MTRK: strdup(%zX) 0 ",dbg_p(s));
  char *ptr = (char *)strdup(s);
  size_t size = ptr ? strlen(ptr)+1 : 0;
  dbg_writeln("%zd %zX",size,dbg_p(ptr));
  return ptr;
}

static inline char *dbg_strndup(const char *s, size_t size, const char *file, int line)
{
  dbg_write("MTRK: strndup(%zX,%zd) 0 ",dbg_p(s),size);
  char *ptr = (char *)strndup(s, size);
  size = ptr ? strlen(ptr)+1 : 0;
  dbg_writeln("%zd %zX",size,dbg_p(ptr));
  return ptr;
}

#define strdup(s)     dbg_strdup(s,__FILE__, __LINE__)
#define strndup(s,n)  dbg_strndup(s,n,__FILE__, __LINE__)

// Check boundaries for the most common functions

static inline char *dbg_strcpy(char *dest, const char *src, const char *file, int line)
{
  size_t size = src? strlen(src)+1 : 0;
  dbg_writeln("MCHK: strcpy(%zX,%zX) %zX %zX",dbg_p(dest),dbg_p(src),dbg_p(dest),dbg_p(dest+size));
  return strcpy(dest,src);
}

static inline char *dbg_strncpy(char *dest, const char *src, size_t size, const char *file, int line)
{
  dbg_writeln("MCHK: strncpy(%zX,%zX,%zd) %zX %zX",dbg_p(dest),dbg_p(src),size,dbg_p(dest),dbg_p(dest+size));
  return strncpy(dest,src,size);
}

static inline char *dbg_strcat(char *dest, const char *src, const char *file, int line)
{
  size_t size = (dest ? strlen(dest) : 0) + (src? strlen(src)+1 : 0);
  dbg_writeln("MCHK: strcat(%zX,%zX) %zX %zX",dbg_p(dest),dbg_p(src),dbg_p(dest),dbg_p(dest+size));
  return strcat(dest,src);
}

static inline char *dbg_strncat(char *dest, const char *src, size_t size, const char *file, int line)
{
  dbg_writeln("MCHK: strncat(%zX,%zX,%zd) %zX %zX",dbg_p(dest),dbg_p(src),size,dbg_p(dest),dbg_p(dest+((dest ? strlen(dest) : 0) + size)));
  return strncat(dest,src,size);
}

static inline void *dbg_memcpy(void *dest, const void *src, size_t size, const char *file, int line)
{
  dbg_writeln("MCHK: memcpy(%zX,%zX,%zd) %zX %zX",dbg_p(dest),dbg_p(src),size,dbg_p(dest),dbg_p((char *)dest+size));
  return memcpy(dest,src,size);
}

static inline void *dbg_memmove(void *dest, const void *src, size_t size, const char *file, int line)
{
  dbg_writeln("MCHK: memmove(%zX,%zX,%zd) %zX %zX",dbg_p(dest),dbg_p(src),size,dbg_p(dest),dbg_p((char *)dest+size));
  return memmove(dest,src,size);
}

static inline void *dbg_memset(void *dest, int c, size_t size, const char *file, int line)
{
  dbg_writeln("MCHK: memset(%zX,%zd) %zX %zX",dbg_p(dest),size,dbg_p(dest),dbg_p((char *)dest+size));
  return memset(dest,c,size);
}

#define malloc(sz)    dbg_malloc(sz,__FILE__, __LINE__)
#define calloc(n,sz)  dbg_calloc(n,sz,__FILE__, __LINE__)
#define realloc(p,sz) dbg_realloc(p,sz,__FILE__, __LINE__)
#define free(p)       dbg_free(p,__FILE__, __LINE__)

#define strcpy(d,s)    dbg_strcpy(d,s,__FILE__, __LINE__)
#define strncpy(d,s,n) dbg_strncpy(d,s,n,__FILE__, __LINE__)
#define strcat(d,s)    dbg_strcat(d,s,__FILE__, __LINE__)
#define strncat(d,s,n) dbg_strncat(d,s,n,__FILE__, __LINE__)
#define memcpy(d,s,n)  dbg_memcpy(d,s,n,__FILE__, __LINE__)
#define memmove(d,s,n)  dbg_memmove(d,s,n,__FILE__, __LINE__)
#define memset(d,c,n)  dbg_memset(d,c,n,__FILE__, __LINE__)

#undef  dbgptr
#define dbgptr(p) dbg_msg("MCHK: ptr %zX", dbg_p(p))

#endif // DEBUG_ALLOC


#endif  // DEBUG

#endif // DBG_H_VER
