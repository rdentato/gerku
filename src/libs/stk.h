/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#ifndef STK_VERSION
#define STK_VERSION 0x0000001B

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#define STK_argcnt(x1,x2,x3,x4,xN, ...) xN
#define STK_argn(...)       STK_argcnt(__VA_ARGS__,4,3,2,1,0)
#define STK_argjoin(x,y)    x ## y
#define STK_argcat(x,y)     STK_argjoin(x,y)
#define STK_varargs(f,...)  STK_argcat(f, STK_argn(__VA_ARGS__))(__VA_ARGS__)

typedef struct stk_s *stk_t;
stk_t stkfree(stk_t s);
stk_t stknew();
void stkdrop(stk_t s);

int32_t stkcount(stk_t s);

#define stkpush(...)     STK_varargs(stk_push, __VA_ARGS__)
#define stk_push1(s)     stk_push(s,0,NULL)
#define stk_push2(s,n)   stk_push(s,n,NULL)
#define stk_push3(s,n,e) stk_push(s,n,e)

void *stk_push(stk_t s, uint16_t sz, void *elm);

#define stktop(...)     STK_varargs(stk_top, __VA_ARGS__)
#define stk_top1(s)     stk_top(s,0,NULL)
#define stk_top2(s,n)   stk_top(s,n,NULL)
#define stk_top3(s,n,e) stk_top(s,n,e)
void *stk_top(stk_t s, int delta, int *elmsize);


#ifdef STK_MAIN

/*
  *stk_t      
  ┏━━━━┓
  ┃    ┃              Each slot is two bytes 
  ┗━┿━━┛        top╭─────────────────────────╮
page│    ┏━━━━━━━━━┿━┓┏━━━━━━━━━━━━━━━━━━━━━━▼━━━━━━┓
    ╰───►┃ ○       ○ ┃┃█░░░█░░░░░░░░░░█░░░░░░█      ┃ 
         ┗━┿━━━━━━━━━┛┗▲━━━━━━━━━━━━━━▲━━━━━━━━━━━━━┛
       prev│           │    ╰── n/2 ─╯│ 
           ▼           │         n (len of element)
                       │
                       ╰─ 0xFFFF(BOTTOM)
*/

#define STK_PAGEBOTTOM   0xFFFE
#define STK_STACKBOTTOM  0xFFFF
#define STK_MAXELMSZ     0xFFF0

#define STK_PAGESIZE  (16*1024)
#define STK_MAXTOP    (STK_PAGESIZE/2)

typedef struct stk_pg_s {
  struct stk_pg_s *prev;
  uint16_t top;
  uint16_t data[STK_MAXTOP+1];     //
} stk_page_t;

typedef struct stk_s {
  stk_page_t *page;
  int32_t     count;  
} *stk_t;

stk_t stknew()
{
  stk_t s = NULL;

  s = malloc(sizeof(struct stk_s));
  if (s) {
    s->count = 0;
    s->page = NULL;
  }
  return s;
}

stk_t stkfree(stk_t s)
{
  if (s) {
    stk_page_t *page = s->page;
    stk_page_t *p;
    while (page) {
      p = page;
      page = page->prev;
      free(p);
    }
    free(s);
  }
  return NULL;
}

void *stk_push(stk_t s, uint16_t sz, void *elm)
{
  uint32_t slots_needed;
  void *ret;

  if (!s) {errno= EINVAL; return NULL;}
  if (sz == 0 || sz > STK_MAXELMSZ) {errno = EINVAL; return NULL;}

  slots_needed = (sz+1)/2;   // Convert to number of uint16_t (in excess)
  slots_needed++ ;           // Add one slot to store the size
 
  if (!s->page ||  ((STK_MAXTOP - s->page->top) < slots_needed)) {
    stk_page_t *p;
    p = malloc(sizeof(stk_page_t));
    if (!p) {errno = ENOMEM; return NULL;}
    p->data[0] = STK_PAGEBOTTOM;
    p->top  = 0;
    p->prev = s->page;
    s->page = p;
  }

  ret= &(s->page->data[s->page->top+1]);
  if (elm) memcpy(ret,elm,sz);

  s->page->top += slots_needed;
  s->page->data[s->page->top] = sz;
  s->count++;
  return ret;
}

void stkdrop(stk_t s)
{
  uint16_t len;

  if (!s || s->count == 0) {errno= EINVAL; return ;}
  
  len = s->page->data[s->page->top] ;

  s->page->top -= (1+(len+1)/2);
  s->count--;

  if (s->page->top == 0) {
    stk_page_t *page = s->page;
    s->page = page->prev;
    free(page);
  }
}

void *stk_top(stk_t s, int delta, int *elmsize)
{
  uint16_t *ret = NULL;
  uint16_t len = 0;
  uint16_t cur;
  stk_page_t *page;
  
  if (!s || s->count == 0) return NULL;

  if (delta < 0) delta = -delta;
  page = s->page;
  cur = page->top;

  for (int i = 0; i <= delta; i++) {
    if (cur == 0) {
      page = page->prev;
      if (!page) { errno = EINVAL; return NULL;  }
      cur = page->top;
    }

    len = page->data[cur];
    cur -= (1+(len+1)/2);
    ret = page->data + cur +1;
  }

  if (elmsize) *elmsize = len;
  return ret;
}

int32_t stkcount(stk_t s)
{
  if (!s) {errno = EINVAL; return 0;}
  return s->count;
}

#endif // STK_MAIN
#endif // STK_VERSION