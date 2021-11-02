/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#include "libs.h"
#include "dict.h"


typedef struct {
  char *str;
  uint16_t len;
} node_t;

vec_t init_stack()
{
  vec_t stack = vecnew(node_t);
  throwif(!stack,ENOMEM);
  return stack;
}

void print_stack(vec_t stack)
{
  node_t *v;

  v=vec(stack);
  fputs("|-> ",stdout);
  for (int k = 0; k<veccount(stack); k++) {
    fputs(v[k].str,stdout); fputc(' ',stdout);
  }
  putchar('\n');
}

static void wipenode(node_t *nd)
{
  throwif(!nd,ERANGE);
  free(nd->str);
  nd->str = NULL;
  nd->len = 0;
}

void wipe_stack(vec_t stack)
{
  node_t *v;
  v=vec(stack);
  for(int k=0; k<veccount(stack); k++) {
    wipenode(v+k);
  }
  veccount(stack,0);
}

vec_t free_stack(vec_t stack)
{
  wipe_stack(stack);
  return vecfree(stack);
}

static int popnode(vec_t stack)
{
  if (veccount(stack)>0) {
    wipenode(vectop(stack));
    vecdrop(stack);
  }
  return 0;
}

static int pushnode(vec_t stack, char *start, int len)
{
  char *term;
  
  term = malloc(len+1);
  throwif(!term, ENOMEM);

  memcpy(term,start,len);
  term[len] = '\0';
  vecpush(stack,&((node_t){term, len}));

  return 0;
}

typedef struct {
   char *name;
   char *(*reduce_f)(vec_t);
} comb_base_t;

char *reduce_del(vec_t stack)
{
  char *ret = NULL;
  if (veccount(stack) > 1) {
    popnode(stack); // del
    popnode(stack); // the other term
  }
  return ret;
}

static char *reduce_dup(vec_t stack)
{
  char *ret = NULL;
  node_t *nd;

  if (veccount(stack) > 1) {
    popnode(stack); // dup
    nd = vectop(stack);
    ret = dupstr(nd->str);
  }
  return ret;
}

static char *reduce_swap(vec_t stack)
{
  char *ret = NULL;
  node_t *nd1;
  node_t *nd2;

  if (veccount(stack) > 2) {
    popnode(stack); // swap
    nd1 = vectop(stack);
    nd2 = vectop(stack,-1);

    ret = nd2->str;
    nd2->str = nd1->str;
    nd2->len = nd1->len;

    nd1->str = NULL; // Avoid free()
    popnode(stack);
  }
  return ret;
}

static char *reduce_concat(vec_t stack)
{
  char *ret = NULL;
  node_t *nd1;
  node_t *nd2;
  int32_t len1;
  int32_t len2;
  char *newstr;
  if (veccount(stack) > 2) {
    nd1 = vectop(stack,-1);
    nd2 = vectop(stack,-2);
    if (nd1->str[0] == '(' && nd2->str[0] == '(') {
      len1 = nd1->len-1;
      len2 = nd2->len-1;
      newstr=malloc(len1 + len2 + 2); // 1 space and 1 \0
      throwif(!newstr,ENOMEM);

      memcpy(newstr,nd2->str,len2);
      newstr[len2] = ' ';

      memcpy(newstr+len2+1,nd1->str+1,len1);
      newstr[len2+len1+1] = '\0';
      
      free(nd2->str);
      nd2->str = newstr;
      nd2->len = len1+len2+1;

      popnode(stack); // concat
      popnode(stack); // nd1
    } 
   
  }
  return ret;
}

static char *reduce_quote(vec_t stack)
{
  char *ret = NULL;
  node_t *nd1;
  int32_t len1;
  char *newstr;
  if (veccount(stack) > 1) {
    popnode(stack); // quote
    nd1 = vectop(stack);
    len1 = nd1->len+2;
    newstr=malloc(len1 + 1); 
    throwif(!newstr,ENOMEM);

    newstr[0] = '(';
    memcpy(newstr+1,nd1->str,len1-2);
    newstr[len1-1] = ')';
    newstr[len1] = '\0';

    free(nd1->str);
    nd1->str = newstr;
    nd1->len = len1;
  }
  return ret;
}

static char *reduce_unquote(vec_t stack)
{
  char *ret = 0;
  node_t *nd1;
  char *newstr;
  if (veccount(stack) > 1) {
    nd1 = vectop(stack,-1);
    if (nd1->str[0] == '(') {
      newstr = nd1->str;
      newstr[nd1->len-1] = '\0';
      for (int k=0; k < nd1->len-2; k++) {
        newstr[k] = newstr[k+1];
      }
      newstr[nd1->len-2] = '\0';

      nd1->str = NULL; // avoid free()
      popnode(stack); // unquote
      popnode(stack); // term

      ret = newstr;
    }
  }
  return ret;
}

static char *reduce_word(vec_t stack, char *word)
{
  char *ret = NULL; 
 _dbgtrc("REDUCE WORD: '%s'",word);

  char **w = search_word(word);
  if (w) {
    int32_t size = 0;
    node_t *nd;
    int nargs =0;
    char *args;
    char *s = *w;
    char *t;
    // check arguments
    while(*s) s++;
    s++;
    nargs = *s++;
    args = s;

    // Check for enough arguments
    if (veccount(stack) <= nargs) return NULL;
    
    size = 2*nargs+16;
    nd = vectop(stack); // word
    node_t *nd1 = nd - nargs;
    // Check args type (quote/term)
    for (int k=0; k< nargs; k++, s++, nd1++) {
      
      if ((*s & 0x80) && (nd1->str[0] != '('))
        return NULL;

      size += nd1->len * (*s & 0x7F);
    }

    // Build expressions
    size += *s++;
   _dbgtrc("size=%d",size);
    ret = malloc(size);
    t = ret;
    int n;
    int l;
    while (*s) {
      if (*s == '@') {
        n = (atoi(++s)-1);
        l = (args[n] & 0x80 )? 1 : 0;
        n = -nargs + n;
        while (isdigit(*s)) s++;
        s--;

        memcpy(t,nd[n].str+l ,nd[n].len - (2*l));

        t += nd[n].len - (2*l);

        //*t++ =' ';
      }
      else *t++ = *s;
      *t = '\0';
      s++;
    }
   _dbgtrc("expr: '%s'",ret);
    // pop
    for (int k=0; k<=nargs; k++)
      popnode(stack);
  }

  return ret;
}


comb_base_t base[] = {
  { "remove", reduce_del} ,
  { "dup", reduce_dup} ,
  { "swap", reduce_swap},
  { "concat", reduce_concat},
  { "quote", reduce_quote},
  { "unquote", reduce_unquote},
  { NULL, NULL}
};

static char *reduce(vec_t stack)
{
  char *ret = NULL;
  node_t *nd;
  comb_base_t *r;

  if (veccount(stack) == 0) return NULL;

  nd = vectop(stack);

  if (nd->str[0] == '(') return NULL;

  ret = reduce_word(stack, nd->str);

  if (ret == NULL) {
    // Search among the "hard-wired" rules
    for (r = base; (r->name); r++) {
      if (strcmp(nd->str,r->name) == 0) {
        ret = r->reduce_f(stack);
        return ret;
      }
    }
  }

  return ret;
}

typedef struct {
   char *ln;
   char *cur;
} evalstr_t;

static int eval_lns(vec_t stack, vec_t lns, int trace)
{
  char *end;
  evalstr_t *curln;
  char *newln;
  int prev_depth;

  while (veccount(lns) > 0) {
    curln = vectop(lns);
    newln = NULL;
   _dbgtrc("CURLN: %p", (void *)curln);
   
    skp("&+s",curln->cur,&(curln->cur));

   _dbgtrc("EVAL: %s (%p)",curln->cur,(void *)(curln->cur));
    while (*(curln->cur)) {
      end = skp("&[A-Za-z_]&*[A-Za-z0-9_-]\1&B\2",curln->cur);
      if (end > curln->cur) {
  //      printf("PUSH: %.*s\n",(int)(end-cur),cur);
        pushnode(stack, curln->cur, (end - curln->cur));
        curln->cur = end;
        skp("&+s",curln->cur,&(curln->cur));

        if (trace) print_stack(stack);
        prev_depth = veccount(stack);

        newln = reduce(stack);
       _dbgtrc("REDUCED: %s",newln?newln:"\"\"");

        if (newln) {
          vecpush(lns,&((evalstr_t){newln, newln}));
         _dbgtrc("PUSHED: %s (%d)",newln,veccount(lns));
          break;
        }
        
        if (trace && veccount(stack) != prev_depth) print_stack(stack);
        
      }
      else {
        fprintf(stderr, "ERROR: Invalid term.\n");
        fprintf(stderr, "%.*s...\n",(int)(curln->cur - curln->ln + 1), curln->cur);
        return 1;
      }
    }
    if (!newln) {
      free(curln->ln);
      curln->ln = NULL;
      vecdrop(lns);
    }
  }
  return 0;
}

int eval(vec_t stack, char *ln, int trace)
{
  int ret=0;
  char *firstln;
  vec_t lns=NULL;

  if (!ln || !*ln) return 0;

  lns = vecnew(evalstr_t);
  throwif(!lns,ENOMEM);

  firstln = dupstr(ln); // Will be freed by eval_lns
  throwif(!firstln,ENOMEM);

  vecpush(lns,&((evalstr_t){firstln,firstln}));
  ret = eval_lns(stack,lns,trace);
  lns = vecfree(lns);
  return ret;
}

