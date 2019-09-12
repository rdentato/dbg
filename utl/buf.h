/* 
**  (C) 2018 by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef BUF_VERSION
#define BUF_VERSION       0x0001001B
#define BUF_VERSION_STR "buf 0.1.1-beta"

typedef struct buf_s  *buf_t;

#define buflen(...)    buf_len(buf_exp(buf_0(__VA_ARGS__,NULL)), \
                               buf_exp(buf_1(__VA_ARGS__,BUF_GETNDX,BUF_GETNDX)))
  
#define bufdel(b,...)  buf_del(b,buf_exp(buf_0(__VA_ARGS__,NULL)), \
                                 buf_exp(buf_1(__VA_ARGS__,1,1)))
  
#define buf(b,...)     buf_(b,buf_exp(buf_0(__VA_ARGS__,0)), \
                              buf_exp(buf_1(__VA_ARGS__,-1,-1)))

#define bufstr(...)    buf_str(buf_exp(buf_0(__VA_ARGS__,NULL)),  \
                               buf_exp(buf_1(__VA_ARGS__,0,0)), \
                               buf_exp(buf_2(__VA_ARGS__,NULL,NULL,NULL)))


buf_t    bufnew(void);
buf_t    buffree(buf_t b);

int      bufshrink(buf_t b, uint32_t size);
uint32_t bufsize(buf_t b);

void     bufins(buf_t b, uint32_t i, char c);
void     bufadd(buf_t b, char c);
char    *bufcat(buf_t b, char *s);
char    *bufcpy(buf_t b, uint32_t i, char *s);
char    *bufgets(buf_t b,uint32_t i, FILE *f);

/**************************************************/
/******* PRIVATE! FOR MAINTENERS EYES ONLY ********/
/**************************************************/

#define BUF_MAXSIZE 0xFFFFFFFF
#define BUF_GETNDX  BUF_MAXSIZE
#define BUF_GETPTR  ((void *)&(buf_dummyptr))

extern int buf_dummyptr;

int      buf_makeroom(buf_t b, uint32_t size);
int      buf_makegap(buf_t b, uint32_t i, uint32_t l);
void     buf_del(buf_t v, uint32_t i, uint32_t l);
char     buf_(buf_t b, uint32_t i,int c);
char    *buf_str(buf_t b, uint32_t i, char *s);

uint32_t buf_len(buf_t b,uint32_t i);

#define buf_exp(x)        x
#define buf_0(x,...)     (x)
#define buf_1(y,x,...)   (x)
#define buf_2(z,y,x,...) (x)

struct buf_s {
  uint32_t sz;
  uint32_t len;
  char *str;
};

#endif // BUF_VERSION

