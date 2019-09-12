/* 
**  (C) 2018 by Remo Dentato (rdentato@gmail.com)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "utl.h"

int n_fail=0, n_pass=0;

#define FLG_QUIET    0x0001
#define FLG_HTML     0x0002
#define FLG_NOTRACE  0x0004
#define FLG_UTF8     0x0008

uint32_t flags = 0;

#define MAX_MATCH 65535

typedef struct watch_s {
   char    *pat;  // Pattern to match
   uint16_t min;  // Minimum required matches
   uint16_t max;  // Maximum allowed matches
   uint16_t cnt;  // Matches count
} watch_t;

#define WATCHMAX 32
typedef struct track_s {
  buf_t   wbuf;            // buffer to hold the patterns
  char   *wfile;           // File and line 
  int     wnum;            // Number of pattern to watch
  watch_t watch[WATCHMAX]; // patterns to watch
} track_t;

#define TRKNESTMAX 10
track_t trk[TRKNESTMAX];
int     tnum=0;

/*  "n-m:pattern"
    n-m min n occurences, up to m
     -m up to m occurences
    n-  at least n occurrences
    n   exactly n occurences (default 1)
    0   pattern must not appear
 */

//******************************************

void trace_start(char *ln)
{
  
  if (tnum >= TRKNESTMAX) return;

  if (trk[tnum].wbuf== NULL) trk[tnum].wbuf = bufnew();
  bufcpy(trk[tnum].wbuf,0,ln+6);
  trk[tnum].watch[0].pat = NULL;

  char *t,*s;
  skp_t *capt = skpnew(8);
  s=bufstr(trk[tnum].wbuf);
  while (s && *s && trk[tnum].wnum < WATCHMAX) {
    t = skp(s,"%>\"%(%*%d%)%(%?%[-+%]%)%(%*%d%):%(%+%[^\"%]%)\"",capt);
    if (t) {
      _dbgmsg("CAP0: %.*s",skplen(capt,0),skpstart(capt,0));
      _dbgmsg("CAP1: %.*s",skplen(capt,1),skpstart(capt,1));
      _dbgmsg("CAP2: %.*s",skplen(capt,3),skpstart(capt,3));
      if (skplen(capt,4) > 0) {
        watch_t *w = &trk[tnum].watch[trk[tnum].wnum];
        w->cnt = 0; w->min = 1;  w->max = MAX_MATCH;
        w->pat = skpstart(capt,4);
       *skpend(capt,4)='\0';
        if (skplen(capt,1) > 0)  w->max = (w->min = atoi(skpstart(capt,1)));
        if (skplen(capt,2) > 0)  w->max = MAX_MATCH;
        if (skplen(capt,3) > 0)  w->max = atoi(skpstart(capt,3));
        trk[tnum].watch[++trk[tnum].wnum].pat = NULL;
      }
      s = t;
    }
    else {
      t = skp(s,"%>\x9%+%.",capt);
      _dbgmsg("FILN: |%.*s|",skplen(capt,0),skpstart(capt,0));
      if (skplen(capt,0) >0) {
        trk[tnum].wfile = skpstart(capt,0);
       *skpend(capt,0) = '\0';
      }
      s = NULL;
    }
  }
  _dbgblk {
    dbgmsg("WATCHING: %d strings @%s",trk[tnum].wnum,trk[tnum].wfile);
    for (int k=0; k<trk[tnum].wnum; k++ ) {
      dbgmsg("%2d-%2d \"%s\"",trk[tnum].watch[k].min, trk[tnum].watch[k].max, trk[tnum].watch[k].pat);
    }
  }
  tnum++;
  skpfree(capt);
}

void trace_line(char *ln)
{
  char *f="";
  skp_t *capt = skpnew(8);

  if (tnum <= 0) return;

  f=skp(ln,"%>\x9%+%.",capt);
  if (skplen(capt,0) > 0) {
    f=skpstart(capt,0); 
    *skpend(capt,0) = '\0';
    _dbgmsg("F:%s",f);
  }

  watch_t *w;
  for (int j=tnum-1; j>=0; j--) {
    for(int k=0; k<trk[j].wnum; k++) {
      w=&trk[j].watch[k];
      _dbgmsg("CHECK: '%s' on '%s'",ln,w->pat);
      if (skp(ln,w->pat)) {
        w->cnt++;
        if (w->cnt > w->max) {
          n_fail++; 
          dbgprt("FAIL: \"%s\" match %d times (max: %d) %s\n", w->pat, w->cnt, w->max, f);
        }
        else if (w->cnt >= w->min) {
          n_pass++; 
          dbgprt("PASS: \"%s\" match %d times (min: %d) %s\n", w->pat, w->cnt, w->min, f);
        }
      }
    }
  }

  skpfree(capt);
}

void trace_end(void)
{
  if (tnum > 0) {
    tnum--;
    for (int k=0; k<trk[tnum].wnum; k++) {
      if (trk[tnum].watch[k].cnt < trk[tnum].watch[k].min) {
        n_fail++; 
        dbgprt("FAIL: \"%s\" match %d times (min: %d) %s\n", trk[tnum].watch[k].pat, trk[tnum].watch[k].cnt, trk[tnum].watch[k].min, trk[tnum].wfile);
      }
    }

    buf(trk[tnum].wbuf,0,'\0');
    trk[tnum].wnum = 0;
  }
}

void trace_init(void)
{
  for (int i=0; i<TRKNESTMAX; i++) {
    trk[i].wbuf=NULL;
    trk[i].wnum = 0;
  }
  tnum=0;
}

int main(int argc, char *argv[])
{
  char *p;
  buf_t linebuf;
  
  trace_init();

  linebuf=bufnew();
  while ((p=bufgets(linebuf,0,stdin))) {
    if (strncmp(p,"STAT: ",6)==0) continue; 
    
    if (!(flags & FLG_QUIET)) dbgprt("%s",p);

         if (strncmp(p,"PASS: ",6)==0) n_pass++; 
    else if (strncmp(p,"FAIL: ",6)==0) n_fail++; 
    else if (strncmp(p,"TEST: ",6)==0) ;
    else if (strncmp(p,"TIME: ",6)==0) ;
    else if (strncmp(p,"TRK[: ",6)==0) { if (!(flags & FLG_NOTRACE)) trace_start(p); }
    else if (strncmp(p,"TRK]: ",6)==0) { if (!(flags & FLG_NOTRACE)) trace_end(); }
    else {                               if (!(flags & FLG_NOTRACE)) dbgclk trace_line(p); }
  }

  dbgprt("STAT: FAIL=%d  PASS=%d",n_fail,n_pass);
  
  buffree(linebuf);

  exit(n_fail != 0 || n_pass == 0);
}

