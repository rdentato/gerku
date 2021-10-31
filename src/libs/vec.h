/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#ifndef VEC_VERSION
#define VEC_VERSION 0x0003000B

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>

#define VEC_argcnt(x1,x2,x3,x4,xN, ...) xN
#define VEC_argn(...)       VEC_argcnt(__VA_ARGS__,4,3,2,1,0)
#define VEC_argjoin(x,y)    x ## y
#define VEC_argcat(x,y)     VEC_argjoin(x,y)
#define VEC_varargs(f,...)  VEC_argcat(f, VEC_argn(__VA_ARGS__))(__VA_ARGS__)

#define VEC_NONDX  -1
#define VEC_AUXNDX -2
#define VEC_ALL    INT32_MAX

#define VEC_FILE   0x01
#define VEC_QUEUE  0x02
#define VEC_MAP    0x04
#define VEC_ARRAY  0x08

typedef struct vec_s {
  uint8_t *vec;
  union {
    uint8_t *p;
    int32_t  i;
    uint32_t u;
  } aux;
  int32_t  cnt;
  int32_t  sze;
  int32_t  fst;
  int16_t  esz;
  uint8_t  flg;
  uint8_t  lgn; 
} *vec_t;

typedef union {
  uint32_t h;
  union {
    uint8_t *s;
    uint64_t u;
  } k;
} veckey_t;

#define vecnew(t) vec_new(sizeof(t))
vec_t vec_new(int32_t esz);
vec_t vecfree(vec_t v);

int   vecdeq(vec_t v);
void *vecenq(vec_t v, void *e);

#define veccount(...)  VEC_varargs(veccount,__VA_ARGS__)
#define veccount1(v)   vec_count(v,-1)
#define veccount2(v,n) vec_count(v,n)

int32_t vec_count(vec_t v,int32_t n);


void *vecget(vec_t v, int32_t n);
void *vecset(vec_t v,int32_t n,void *e);

#define vecpush(v,e)  vecset(v, VEC_NONDX, e)

#define vectop(...)   VEC_varargs(vec_top,__VA_ARGS__)
#define vec_top1(v)   vec_top(v,0)
#define vec_top2(v,n) vec_top(v,n)

void *vec_top(vec_t v, int32_t n);

#define vecdrop(...)  VEC_varargs(vec_drop,__VA_ARGS__)
#define vec_drop1(v)   vec_drop(v,1)
#define vec_drop2(v,n) vec_drop(v,n)

int vec_drop(vec_t v, int32_t n);

#define vechead(v)    vecget(v,VEC_AUXNDX);
#define vectail(v)    vectop(v)

static inline void *vec(vec_t v)
{ return (v? v->vec : NULL); }

static inline int vecclean(vec_t v)
{ return (v && (v->cnt = v->fst = v->flg = 0)); }

#define vecauxptr(...) VEC_varargs(vec_aux_ptr,__VA_ARGS__)
#define vec_aux_ptr1(v)   vec_aux_getp(v)
#define vec_aux_ptr2(v,p) vec_aux_setp(v,p)

static inline void *vec_aux_getp(vec_t v)
{ return (v? v->aux.p : NULL); }

static inline void *vec_aux_setp(vec_t v, void *p)
{ return (v? (v->aux.p = (void *)p) : NULL); }

#define vecauxint(...) VEC_varargs(vec_aux_int,__VA_ARGS__)
#define vec_aux_int1(v)   vec_aux_geti(v)
#define vec_aux_int2(v,i) vec_aux_seti(v,i)

static inline int32_t vec_aux_geti(vec_t v)
{ return (v? v->aux.i : 0); }

static inline int32_t vec_aux_seti(vec_t v, int32_t i)
{ return (v? (v->aux.i = i) : 0); }

#define vecauxuint(...) VEC_varargs(vec_aux_uint,__VA_ARGS__)
#define vec_aux_uint1(v)   vec_aux_getu(v)
#define vec_aux_uint2(v,u) vec_aux_setu(v,u)

static inline uint32_t vec_aux_getu(vec_t v)
{ return (v? v->aux.u : 0); }

static inline uint32_t vec_aux_setu(vec_t v, uint32_t u)
{ return (v? (v->aux.u = u) : 0); }


#define vecconst(t,...) &((t){__VA_ARGS__})

void *vec_ptr(void *ptr);
#define vecvalue(t,p) (t)(*(vec_ptr(p)));

int32_t vecwrite(vec_t v, char *s, int32_t n);
int32_t vecputs(vec_t v, char *s);
int32_t vecprintf(vec_t v, char *fmt , ...);


#ifdef VEC_MAIN

void *vec_ptr(void *ptr)
{ 
  static char ptrnull[16] = {0};
  errno = 0;
  if (ptr == NULL) { errno=EINVAL; ptr=ptrnull; }
  return ptr;
}

vec_t vec_new(int32_t esz)
{
  vec_t v = malloc(sizeof(struct vec_s));
  if (v) {
    v->vec = NULL;
    v->esz = esz;  v->sze = 0;  
    v->cnt = 0;  v->fst = 0;  v->flg = 0;
  }
  return v;
}

vec_t vecfree(vec_t v)
{
  if (v) {
    free(v->vec);
    v->vec = NULL;
    v->esz = 0;  v->sze = 0; 
    v->cnt = 0;  v->fst = 0;
    free(v);
  }
  return NULL;
}

static int8_t *get_elm(vec_t v,int32_t n)
{
  int32_t new_sze;
  uint8_t *new_vec;

  if (!v) {errno = EINVAL; return NULL;}
  if (n == VEC_NONDX) n = v->cnt;
  if (n >= v->sze) {
    new_sze = v->sze ? v->sze : 4;
    while (new_sze <= n) {
      new_sze  = (new_sze + (new_sze/2));
    }
    new_sze += (new_sze & 1);

    new_vec = realloc(v->vec, new_sze * v->esz);
    if (new_vec) {
      v->vec = new_vec;
      v->sze = new_sze;
    }
    else { errno = ENOMEM; return NULL; }
  }
  return (int8_t *)((v->vec) + (n * v->esz));
}

void *vecset(vec_t v, int32_t n, void *e)
{
  int8_t *elm;

  if (!v || !e) { errno = EINVAL; return NULL; }
  if (n==VEC_NONDX) n = veccount(v);
  elm = get_elm(v,n);
  if (elm) {
    memcpy(elm,e,v->esz);
    if (n >= v->cnt) v->cnt = n+1;
  }
  return elm;
}

void *vecget(vec_t v, int32_t n)
{ 
  if (v) {
    if (n == VEC_NONDX) n = v->cnt-1;
    if (n >= 0) return (v->vec) + (n * (v->esz));
  }
  errno = ESRCH;
  return NULL;
}

int vec_drop(vec_t v, int32_t n)
{
  if (!v || (v->cnt == 0)) return 0;
  if (n >= v->cnt) n = v->cnt;
  return (v->cnt -= n);
}

void *vec_top(vec_t v, int32_t n)
{
  int32_t k;
  if (n<0) n=-n;
  if (v) {
    k = (v->cnt -1) - n;
    if (k >= 0) return vecget(v,k);
  }
  errno = ESRCH;
  return NULL;
}

int32_t vec_count(vec_t v, int32_t newcnt) 
{
  if (!v) { errno = EINVAL; return 0; }
  if (v->flg & VEC_QUEUE) {
    if (v->fst < v->cnt) return v->cnt - v->fst;
    else { v->fst = 0; v->cnt = 0; }
  }
  else {
    if (newcnt >= 0) v->cnt = newcnt;
  }
  return v->cnt;
} 

/* Queues are using a vector
                v-- cnt points to the tail
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | |B|C|D|E|F| | | | | | | | | | | 
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ^-- fst points to the head
*/ 
// Enque

static int vec_isnot_QUEUE(vec_t v)
{
  if (!v) { errno = EINVAL; return 1; }
  if (v->vec == NULL) { v->flg = VEC_QUEUE; v->fst = 0; v->cnt = 0; }
  if (!(v->flg & VEC_QUEUE)) { errno = EINVAL; return 1; }
  return 0;
}

void *vecenq(vec_t v, void *e)
{
  if (!e || vec_isnot_QUEUE(v)) { errno = EINVAL; return NULL; }

  if ((v->fst > (v->cnt / 2)) && (v->cnt >= v->sze)) {
    memmove(v->vec, v->vec+(v->fst * v->esz), (v->cnt - v->fst) * v->esz);
    v->cnt -= v->fst; v->fst = 0;
  }
  return vecset(v, VEC_NONDX, e);
}

// Deque (delete from head)
int vecdeq(vec_t v) 
{
  if (vec_isnot_QUEUE(v)) return 0;
  v->fst++; 
  if (v->fst >= v->cnt) {v->fst = 0; v->cnt = 0;}
  return (v->cnt - v->fst);
}

// FILE like
#if 0 // TODO

static int vec_isnot_FILE(vec_t v)
{
  if (!v) { errno = EINVAL; return 1; }
  if (v->vec == NULL) { v->flg = VEC_FILE; v->fst = 0; v->cnt = 0; v->sze = 0; v->esz = 1; }
  if (!(v->flg & VEC_FILE)) { errno = EINVAL; return 1; }
  return 0;
}

int32_t vecprintf(vec_t v, char *fmt , ...)
{
  if (vec_isnot_FILE(v)) return -1;

  char *end, *dst;
  int32_t max_n;
  
  va_list args;

  int32_t n;

  max_n = v->sze - v->fst;
  
  if ((n < 32) & !(end = get_elm(v,v->fst + 32))) 
    return -1;

  while (1) {
    max_n = v->sze - v->fst;
    if (!(dst = get_elm(v,v->fst))) return -1;
    va_start(args, fmt);
    n = vsnprintf(dst, max_n, fmt, args);
    va_end(args);

    if (n < max_n) break;

    if (!(end = get_elm(v,(v->fst + 2*n)))) return -1;
  }

  v->fst += n;
  if (v->fst > v->cnt) v->cnt = v->fst;
  return v->fst;
}

int32_t vecputs(vec_t v, char *s)
{
  if (vec_isnot_FILE(v)) return -1;

  if (vecwrite(v,s,strlen(s)+1) <= 0) return -1;
  v->fst -= 1;
  return v->fst;
}

int32_t vecwrite(vec_t v, char *s, int32_t n)
{
  if (vec_isnot_FILE(v)) return -1;
  char *end;
  char *dst;
  
  if (!(end = get_elm(v,v->fst+n))) return -1;
  if (!(dst = get_elm(v,v->fst))) return -1;
  memcpy(dst,s,n);
  v->fst += n; 
  if (v->fst >= v->cnt) v->cnt = v->fst;
  return v->fst;
}

int32_t vecfread(vec_t v, int32_t len, FILE *f)
{
  if (vec_isnot_FILE(v)) return -1;
  return v->fst;
}

int32_t vecseek(vec_t v, int32_t pos)
{
  if (vec_isnot_FILE(v)) return -1;

  return v->fst;
}

int32_t vecpos(vec_t v)
{
  if (vec_isnot_FILE(v)) return -1;

  return v->fst;
}

// Mapping (hashtable)
int vec_isnot_MAP(vec_t v)
{
  if (!v) { errno = EINVAL; return 0; }
  if (v->vec == NULL) v->flg = VEC_MAP;
  if (!(v->flg & VEC_MAP)) { errno = EINVAL; return 0; }
  return 0;
}

void *vec_map(vec_t v, void *e)
{
  if (!e || vec_isnot_MAP(v)) { errno = EINVAL; return NULL; }
  if (v->vec == NULL) v->flg = VEC_MAP;
  if (!(v->flg & VEC_MAP)) { errno = EINVAL; return NULL; }

  return NULL;
}

int vec_unmap(vec_t v, veckey_t *k)
{
  return 0;
}

void *vecmapint(vec_t v, int32_t k, void *e)
{
  return NULL;
}

void *vecmapuint(vec_t v, uint32_t k, void *e)
{
  return NULL;
}

void *vecmapstr(vec_t v, char *k, void *e)
{
  return NULL;  
}

void *vec_search(vec_t v,veckey_t *k)
{
  return NULL;
}
#endif // TODO

#endif // VEC_MAIN
#endif // VEC_VERSION
