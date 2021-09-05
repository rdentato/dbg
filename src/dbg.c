/* 
**  (C) 2018 by Remo Dentato (rdentato@gmail.com)
*/

/*
   a alphabetic char
   d digit
   x hexdigit
   s space
   . any

  &a
  &+a
  &*a
  &!a

  &.

  &[a-dC-F]


*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "dbg.h"

int32_t n_fail=0;
int32_t n_pass=0;

#define FLG_QUIET    0x0001
#define FLG_HTML     0x0002
#define FLG_NOTRACE  0x0004
#define FLG_UTF8     0x0008
#define FLG_RESULTS  0x0010

uint32_t flags = 0x00000000;

// For each track
#define MAX_TRK_PATS 32
#define MAX_TRK_PATS_STR 256-(MAX_TRK_PATS*2)   
typedef struct trk_s {
    char   string[MAX_TRK_PATS_STR];
   uint8_t pos[MAX_TRK_PATS];
   uint8_t found[MAX_TRK_PATS];
} *trk_t;

#define MAX_TRK_NEST 4

struct trk_s trk_stack[MAX_TRK_NEST];
int trk_cur = 0;

char *trk_pat_start(int8_t n)
{
  int8_t pos;
  trk_t trk;
  if ((n < MAX_TRK_PATS) || (trk_cur < 0)) {
    trk = &(trk_stack[trk_cur]);
    pos = trk->pos;
    if (pos > 0) return trk->string+trk->pos;
  }
  return NULL;
}

#define pat_start(n) (((t)->pat_string)+pat_pos(n))


// set global flags and return the first non-flag argument
int setflags(int argc, char *argv[])
{

}

int main(int argc, char *argv[])
{
  
  exit(n_fail != 0 || n_pass == 0);
}


