/*
**  (C) 2018 by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#include "buf.h"
#include "dbg.h"

int buf_dummyptr=0;

int bufshrink(buf_t b, uint32_t size)
{
  if (!b || b->sz < size) return 0;
  if (size == 0) {
    free(b->str);
    b->sz = 0; b->len = 0;
    b->str = NULL; 
  }
  else {
    char *new_str;
    new_str = realloc(b->str, size);
  
    if (new_str == NULL) return 0;
  
    b->str = new_str;
    b->sz = size;
    if (b->len >= b->sz) {
      b->len = b->sz-1;
      b->str[b->len] = '\0';
    }
  }
  return 1;
}

int buf_makeroom(buf_t b, uint32_t size)
{
  uint32_t new_size = 2;
  char    *new_str = NULL;

  dbginf("entering makeroom(%d)",size);
  if (b == NULL || size == BUF_MAXSIZE) return 0;
  if (size == 0) size = 2;
  if (b->sz >= size) return 1;
  new_size  = size;
  new_size += new_size / 2;
  new_size += new_size & 1;
  _dbgchk(new_size > size,"Size: %d NewSize %d",size, new_size);
  _dbginf("newsize: %d",new_size);
  new_str = realloc(b->str, new_size);
  if (new_str == NULL) return 0;
  b->str = new_str;
  b->sz = new_size;
  return 1;
}

int buf_makegap(buf_t b, uint32_t i, uint32_t l)
{
  dbginf("entering makgap(%d,%d)",i,l);
  if (!buf_makeroom(b, b->len+l)) return 0;
  
  if (i < b->len) {
    /*                     __l__
    **  ABCDEFGH       ABC/-----\DEFGH
    **  |  |    |      |  |      |    |
    **  0  i    len    0  i     i+l   len+l */  
    memmove(b->str+(i+l), b->str+i, b->len-i);
    b->len += l;
    b->str[b->len] = '\0';
  }
  return 1;
}

void buf_del(buf_t b, uint32_t i, uint32_t l)
{
  dbginf("buf_del: %d %d",i,l);
  if (i < b->len) {
    if (i+l >= b->len) b->len = i; /* Just drop last elements */
    else {
      /*       __l__                                
      **    ABCdefghIJKLM              ABCIJKLM     
      **    |  |    |    |             |  |    |    
      **    0  i   i+l   len           0  i    len-l  */
      memmove(b->str+i, b->str+(i+l), b->len-(i+l));
      b->len -= l;
    }
    b->str[b->len] = '\0';
  }
}

uint32_t buf_len(buf_t b, uint32_t i)
{
  if (!b) return 0;
  if ((i != BUF_GETNDX) && buf_makeroom(b,i+1)) b->len = i;
  b->str[b->len] = '\0';
  return b->len;
}  

uint32_t bufsize(buf_t b)
{ return b?b->sz:0; }


/** Strings **/

char *bufcpy(buf_t b, uint32_t i, char *s)
{ 
  if (b == NULL || s == NULL) return NULL;
  int l = strlen(s);
  if (!buf_makeroom(b,i+l+1)) return NULL;
  strcpy(b->str+i,s);
  b->len = i+l;
  b->str[b->len]=0;
  return b->str+i;
}

char *bufcat(buf_t b, char *s)
{ return bufcpy(b,b->len,s); }

char *buf_str(buf_t b, uint32_t i, char *s)
{ 
  if (b == NULL) return NULL;
  if (s != NULL) {
    if (i >= b->len) return bufcpy(b,b->len,s);
    int l = strlen(s);
    if (buf_makegap(b,i,l)) memcpy(b->str+i,s,l);
  }
  if (i > b->len) i=b->len;
  return b->str+i;
}

/** Single char **/

void bufins(buf_t b, uint32_t i, char c)
{ if (buf_makegap(b,i,1)) b->str[i] = c; }

char buf_(buf_t b, uint32_t i,int c)
{
  if (c>=0) {
    if (buf_makeroom(b,i+1)) {
       b->str[i] = (char)c;
       if (i >= b->len) {
         b->len = i+(c!='\0');
         b->str[b->len] = '\0';
       }
    }
  }
  if (i>=b->len) return '\0';
  return b->str[i];
}

void bufadd(buf_t b, char c)
{ buf_(b,b->len,c); }

buf_t bufnew(void)
{
  _dbginf("entering bufnew(%d)",sz);
  buf_t b = malloc(sizeof(*b));
  if (b) {
    b->len = 0; b->sz  = 0; 
    b->str = NULL;
  }
  return b;
}

void bufclr(buf_t b)
{
  if (b) {
    if (b->str) free(b->str);
    b->len = 0; b->sz = 0; 
    b->str = NULL;
  }
}

buf_t buffree(buf_t b)
{
  if (b) {
    if (b->str) free(b->str);
    b->str = NULL; b->sz = 0; b->len = 0;
    free(b);
  }
  return NULL;
}

char *bufgets(buf_t b,uint32_t i, FILE *f)
{
  char *s,*e;
  uint32_t ii = i;
  int n=0;

  dbginf("ENTERING bufgets %d",i);
  buf_makeroom(b,i+16);
  dbginf("Len: %d size: %d",buflen(b),bufsize(b));
  dbginf("START: buf[%d] == '%02X' (ln:%d sz:%d)",b->sz-1,'X',b->len,b->sz);
  do {
    b->str[i] = '\0';
    b->len = i;
    b->str[b->sz-1] = 'X';
    s=fgets(b->str+i,(b->sz-i),f);
    dbginf("FGETS: buf[%d] == '%02X' (ln:%d sz:%d)",b->sz-1,b->str[b->sz-1],b->len,b->sz);
    if (!s || b->str[b->sz-1] == 'X') break;
    dbgreq(b->str[b->sz-1] == '\0',"Unexpected change");
    i = b->sz-1;
    buf_makeroom(b,b->sz+16);
    n++;
  } while(1);
  if (s == NULL && n==0) return NULL;
  e = b->str+b->sz;
  s = b->str+b->len;
  while (s<e && *s) s++;
  b->len = s-b->str;
  b->str[b->len] = '\0';

  dbginf("EXITING Len: %d size: %d",buflen(b),bufsize(b));
  dbgreq(b->len < b->sz,"Len > Size!");
  return b->str+ii; 
}
