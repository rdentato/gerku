/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

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

vec_t dict = NULL;

// Don't trust strdup exists
char *dupstr(char *s)
{
  char *new_s;
  int n = strlen(s)+1;
  if ((new_s = malloc(n))) memcpy(new_s,s,n);
  return new_s;
}

// ($) $ pippo = $1 $1 dip $2 unquote
int add_word(char *source)
{
  char *df = source;
  
  char *name ;
  char *end ;

  skp("&+s",df,&df);
  printf("Defining %s\n",df);

  name = skp("&>&[A-Za-z_]&*[A-Za-z0-9_-]&@&*s=",df, &end);
  
  if (end <= name) {
    printf("ERROR IN DEF\n");
    return 0;
  }
  
  dbgtrc("DEF: name: %.*s",(int)(end-name),name);

  // Get args
  // Get name

  // Get def

  return 0;
}

#define chkcmd(s,l,n) ((strncmp(s,l,n) == 0) && ((l[n] == '\0') || isspace(l[n])))
int command(char *ln)
{
  if (chkcmd("list",ln,4)) {
    fputs("($) ($) concat = ($1 $2)\n",stderr );
    fputs("$ dup = $1 $1\n",stderr );
    fputs("$ quote = ($1)\n",stderr );
    fputs("$ remove =\n",stderr );
    fputs("$ $ swap = $2 $1\n",stderr );
    fputs("($) unquote = $1\n",stderr );

    fputs("c = concat\n",stderr );
    fputs("d = dup\n",stderr );
    fputs("q = quote\n",stderr );
    fputs("r = remove\n",stderr );
    fputs("s = swap\n",stderr );
    fputs("u = unquote\n",stderr );
    return 0;
  }

  if (chkcmd("trace",ln,5)) {
    trace = !trace;
    printf("Trace is %s\n","OFF\0ON" + (trace*4));
    return 0;
  }

  if (chkcmd("print",ln,5)) {
    return 0;
  }

  if (chkcmd("def",ln,3)) {
    add_word(ln+3);
    return 0;
  }

  if (chkcmd("wipe",ln,4)) {
    return WIPE;
  }

  if (chkcmd("quit",ln,4)) {
    return QUIT;
  }

  if (!chkcmd("help",ln,4)) {
    fputs("Available commands:\n",stderr);
  }

  fputs("  !help        this help\n",stderr );
  fputs("  !list        list of defined words\n",stderr );
  fputs("  !load file   load definitions from file\n",stderr );
  fputs("  !save file   save definitions to file\n",stderr );
  fputs("  !print       print current stack\n",stderr );
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

char *reduce_dup(vec_t stack)
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

char * reduce_swap(vec_t stack)
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

char *reduce_concat(vec_t stack)
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

char *reduce_quote(vec_t stack)
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

int eval(vec_t stack, char *ln);

char *reduce_unquote(vec_t stack)
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

comb_base_t base[] = {
  { "remove", reduce_del} ,
  { "dup", reduce_dup} ,
  { "swap", reduce_swap},
  { "concat", reduce_concat},
  { "quote", reduce_quote},
  { "unquote", reduce_unquote},
  { NULL, NULL}
};

char *reduce(vec_t stack)
{
  char *ret = NULL;
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


typedef struct {
   char *ln;
   char *cur;
} evalstr_t;

int eval_lns(vec_t stack, vec_t lns)
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

        if (trace) prtstack(stack);
        prev_depth = veccount(stack);

        newln = reduce(stack);
       _dbgtrc("REDUCED: %s",newln?newln:"\"\"");

        if (newln) {
          vecpush(lns,&((evalstr_t){newln, newln}));
         _dbgtrc("PUSHED: %s (%d)",newln,veccount(lns));
          break;
        }
        
        if (trace && veccount(stack) != prev_depth) prtstack(stack);
        
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

int eval(vec_t stack, char *ln)
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
  ret = eval_lns(stack,lns);
  vecfree(lns);
  return ret;
}

// REPL ******************

#ifdef USE_LINENOISE

#define get_line(l) (l = linenoise("gerku> "))
char *buf=NULL;
#define clear_line(l) (l? free(l) : 0, l = NULL)

#else 

#define MAXLINE 256
char buf[MAXLINE];
#define get_line(l) (fprintf(stdout,"gerku> "),fgets(l,MAXLINE,stdin))
#define clear_line(l) (*l='\0');

#endif

int main(int argc, char *argv[])
{
  int ret = 0;
  char *line = buf;
  char *ln;
  vec_t stack = NULL;

  dict = vecnew(node_t);

  try {
 
    stack = vecnew(node_t);
    throwif(!stack,ENOMEM);

    printf("GERKU 0.0.1-beta\nType ! for available commands.\n");
    prtstack(stack);

    while((ret != QUIT) && (get_line(line) != NULL)) {
      ln = line;
      skp("&+s",ln,&ln);

      ret = (*ln == '!')? command(ln+1) : eval(stack,ln);

      clear_line(line);
      if (ret == WIPE) wipestack(stack);
      prtstack(stack);
    }
    
  }
  catchall {
    thrownerr("Unexpected error");
    abort();
  }

  vecfree(dict);
  vecfree(stack);
  clear_line(line);

  return (0);
}

