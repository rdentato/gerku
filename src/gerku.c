/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/


#define LIBS_MAIN

#ifndef DEBUG
#define DEBUG DEBUG_TEST
#endif

#include "gerku.h"

#define QUIT 255
#define WIPE 254

int trace = 0;

typedef struct {
  char *str;
  uint16_t len;
} node_t;

typedef struct {
  char *word;

} word_t;


int command(char *ln)
{
  if (strncmp("list",ln,4) == 0) {
    fputs("($A) ($B) concat = ($A $B)\n",stderr );
    fputs("$A dup = $A $A\n",stderr );
    fputs("$A quote = ($A)\n",stderr );
    fputs("$A remove =\n",stderr );
    fputs("$A $B swap = $B $A\n",stderr );
    fputs("($A) unquote = $A\n",stderr );
    fputs("c = concat\n",stderr );
    fputs("d = dup\n",stderr );
    fputs("q = quote\n",stderr );
    fputs("r = remove\n",stderr );
    fputs("s = swap\n",stderr );
    fputs("u = unquote\n",stderr );
    return 0;
  }

  if (strncmp("trace",ln,4) == 0) {
    trace = !trace;
    printf("Trace is %s\n","OFF\0ON" + (trace*4));
    return 0;
  }

  if (strncmp("wipe",ln,4) == 0) {
    return WIPE;
  }

  if (strncmp("quit",ln,4) == 0) {
    return QUIT;
  }

  if (strncmp("help",ln,4) != 0) {
    fputs("Available commands:\n",stderr);
  }

  fputs("  !help        this help\n",stderr );
  fputs("  !list        list of defined words\n",stderr );
  fputs("  !load file   load definitions from file\n",stderr );
  fputs("  !save file   save definitions to file\n",stderr );
  fputs("  !stack       print current stack\n",stderr );
  fputs("  !trace       toggle reduction tracing\n",stderr );
  fputs("  !quit        exit the repl\n",stderr );
  fputs("  !wipe [all]  wipe the stack [and the dictionary]\n",stderr );

  return 0;
}

void prtstack(vec_t stack)
{
  node_t *v;

  v=vec(stack);
//  fprintf(stderr,"------\n");
//  for(int k=veccount(stack); k>0; k--) {
//    fprintf(stderr,"%03d %.*s\n",k-1,v[k-1].len,v[k-1].str);
//  }
  fputs("|-> ",stdout);
  for (int k = 0; k<veccount(stack); k++) {
    fputs(v[k].str,stdout); fputc(' ',stdout);
  }
  putchar('\n');
}

void wipenode(node_t *nd)
{
  throwif(!nd,ERANGE);
  free(nd->str);
  nd->str = NULL;
  nd->len = 0;
}

void wipestack(vec_t stack)
{
  node_t *v;
  v=vec(stack);
  for(int k=0; k<veccount(stack); k++) {
    wipenode(v+k);
  }
  veccount(stack,0);
}

int popnode(vec_t stack)
{
  if (veccount(stack)>0) {
    wipenode(vectop(stack));
    vecdrop(stack);
  }
  return 0;
}

int pushnode(vec_t stack, char *start, int len)
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
   int (*reduce_f)(vec_t);
} comb_base_t;

int reduce_del(vec_t stack)
{
  int ret = 0;
  if (veccount(stack) > 1) {
    popnode(stack); // del
    popnode(stack); // the other term
    ret=1;
  }
  return ret;
}

int reduce_dup(vec_t stack)
{
  int ret = 0;
  node_t *nd;

  if (veccount(stack) > 1) {
    popnode(stack); // dup
    nd = vectop(stack);
    pushnode(stack, nd->str, nd->len ); // the other term
  }
  return ret;
}

int reduce_swap(vec_t stack)
{
  int ret = 0;
  node_t *nd1;
  node_t *nd2;
  node_t ndtmp;

  if (veccount(stack) > 2) {
    popnode(stack); // swap
    nd1 = vectop(stack);
    nd2 = vectop(stack,-1);
    ndtmp = *nd1;
    *nd1 = *nd2;
    *nd2 = ndtmp;
  }
  return ret;
}

int reduce_concat(vec_t stack)
{
  int ret = 0;
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

int reduce_quote(vec_t stack)
{
  int ret = 0;
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

int eval(vec_t stack, char *ln);

int reduce_unquote(vec_t stack)
{
  int ret = 0;
  node_t *nd1;
  char *newstr;
  if (veccount(stack) > 1) {
    nd1 = vectop(stack,-1);
    if (nd1->str[0] == '(') {
      newstr = nd1->str;
      newstr[nd1->len-1] = '\0';
      nd1->str = NULL;
      popnode(stack); // unquote
      popnode(stack); // term
      eval(stack, newstr+1);
      free(newstr);
    }
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

int reduce(vec_t stack)
{
  int ret = 0;
  node_t *nd;
  comb_base_t *r;

  if (veccount(stack) == 0) return 0;

  nd = vectop(stack);
  for (r = base; (r->name); r++) {
    if (strcmp(nd->str,r->name) == 0) {
      ret = r->reduce_f(stack);
      return ret;
    }
  }
  return ret;
}

int eval(vec_t stack, char *ln)
{
  int ret = 0;
  char *start = ln;
  char *end;
  skp("&+s",start,&start);

  while (*start) {
    
    end = skp("&+[A-Za-z_]&*[A-Za-z0-9_-]\1&B\2",start);
    if (end > start) {
//      printf("PUSH: %.*s\n",(int)(end-start),start);
      pushnode(stack, start, (end-start));
      if (trace) prtstack(stack);
      if (reduce(stack) && trace) prtstack(stack);
      start = end;
      skp("&+s",start,&start);
    }
    else {
      fprintf(stderr, "ERROR: Invalid term.\n");
      fprintf(stderr, "%.*s...\n",(int)(start-ln+1),ln);
      ret = 1;
      break;
    }
  }
  return ret;
}


#define MAXLINE 256
int main(int argc, char *argv[])
{
  int ret = 0;
  char *line = NULL;
  char *ln;
  vec_t stack = NULL;
 
  try {
 
    stack = vecnew(node_t);
    throwif(!stack,ENOMEM);

    printf("GERKU 0.0.1-beta\nType ! for available commands.\n");
    prtstack(stack);

    while((ret != QUIT) && (line = linenoise("gerku> ")) != NULL) {
      ln = line;
      skp("&+s",ln,&ln);

      ret = (*ln == '!')? command(ln+1) : eval(stack,ln);

      free(line); line = NULL;
      if (ret == WIPE) wipestack(stack);
      prtstack(stack);
    }
    
  }
  catchall {
    thrownerr("Unexpected error");
    abort();
  }

  vecfree(stack);
  if (line) free(line);

  return (0);
}

