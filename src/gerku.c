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

// Don't trust strdup exists
char *dupstr(char *s)
{
  char *new_s;
  int n = strlen(s)+1;
  if ((new_s = malloc(n))) memcpy(new_s,s,n);
  return new_s;
}

typedef struct {
  char *str;
  uint16_t len;
} node_t;

// DICTIONARY ******

vec_t dict = NULL;

char **search_word(char *word)
{
  char **v=vec(dict);
  for (int k=0; k<veccount(dict); k++) {
    if (strcmp(v[k],word) == 0) return v+k;
  }
  return NULL;
}



/* 
             ╭── n ──╮
  ┌──────┬─┬─┬───────┬─┬──────┬─┐
  │ word │0│n│ ┆ ┆ ┆ │l│ body │0│
  └──────┴─┴─┴───────┴─┴──────┴─┘
            ▲ ▲ 
            │ ╰─ # of occurences in the body 
            │    msb set means it is a quote
            │
            ╰─ number of arguments 

*/

// (@) @ pippo = @1 @1 dip @2 unquote
int add_word(char *def)
{
  int nargs;

  char *dict_str;

  char *name_start, *name_end;
  char *args_start, *args_end;
  char *body_start, *body_end;
   
  uint8_t name_size;
  //uint8_t args_size;
  uint8_t body_size;

  if (!def || !*def) return 0;
  _dbgtrc("Defining %s",def);
  
  // Will surely be enough;
  dict_str = malloc(strlen(def)+16);
  
  nargs = 0;
  args_start = skp("&+s",def);
  args_end = args_start;
  while(1) {
     dict_str[nargs] = (*args_end == '@'? 0x00 : 0x80);
     args_end = skp("(&*s@&*s)\1@\2",args_end);
     if (errno) break;
     nargs++;
    _dbgtrc("ARG: %d '%02X'",nargs,(uint8_t)dict_str[nargs-1]);
     skp("&+s",args_end, &args_end);
  }
  
  if (nargs > 100) {
    printf("Error: too many arguments.\n");
    free(dict_str);
    return 0;
  }

  name_start = args_end;
  name_end = skp("&[A-Za-z_]&*[A-Za-z0-9_-]&@&*s=",name_start);
  
  if (name_end <= name_start) {
    printf("Error: Syntax error in definition.\n");
    free(dict_str);
    return 0;
  }
  
  body_start = skp("&*s=&*s",name_end);
  body_end = skp("&+.",body_start);

  int n;
  for (char *s=body_start; s < body_end; s++) {
    if (*s == '@') {
      n = atoi(s+1);
      if (n < 1 || nargs < n) {
        printf("Error: Invalid reference in definition.\n");
        free(dict_str);
        return 0;
      }
      n--;
      dict_str[n] = ((uint8_t)dict_str[n]+1);
    }
  }

 _dbgblk {
    for (int k=0; k<nargs; k++) {
      dbgtrc("ARG[%d] %02X",k,(uint8_t)dict_str[k]);
    }
  }

  while (args_end > args_start && isspace(args_end[-1]))
    args_end--;

  name_size = (uint8_t) (name_end - name_start);
  //args_size = (uint8_t) (args_end - args_start);
  body_size = (uint8_t) (body_end - body_start);

 _dbgtrc("DEF: name: '%.*s'",name_size, name_start);
 _dbgtrc("DEF: args: '%.*s' (%d)",args_size, args_start,nargs);
 _dbgtrc("DEF: body: '%.*s'",body_size, body_start);

  // move args after the name

  for (int k=nargs-1; k >=0 ; k--) { 
    dict_str[(name_size+2)+k] = dict_str[k];
  }
  dict_str[name_size+1] = (int8_t)nargs;

  memcpy(dict_str,name_start,name_size);
  dict_str[name_size]= '\0' ;

  dict_str[name_size+1+1+nargs] = (int8_t)body_size;
  memcpy(dict_str+name_size+1+1+nargs+1, body_start,body_size+1);

  char **w = search_word(dict_str);

  if (!w) {
    vecpush(dict, &dict_str);
  }
  else {
    free(*w);
    *w = dict_str;
  }

  // if word already exists
  //   replace def
  // else
  //   add definition                      

  return 0;
}

int del_word(char *word)
{
  char **w;
  char **x;

  while (isspace(*word)) word++;
  w = search_word(word);
  if (w) {
    free(*w); *w = NULL;
    x = vectop(dict);
    *w = *x;
    vecdrop(dict);
  }
  return 0;
}

int list_words()
{
  char **v = vec(dict);
  char *s;
  int nargs;

  for (int k=0; k<veccount(dict); k++) {
    s=v[k];
    while (*s) s++;
    s++;
    nargs = *s++;
    for (int j=0; j<nargs; j++) {
      printf("%s ", ( *s & 0x80 ? "(@)" : "@"));
      s++;
    }
    
    printf("%s =",v[k]);
    s++; // len
    if (*s) printf(" %s",s);
    putc('\n',stdout);
  }
  return 0;
}

// COMMANDS ****

#define chkcmd(s,l,n) ((strncmp(s,l,n) == 0) && ((l[n] == '\0') || isspace(l[n])))
int command(char *ln)
{
  if (chkcmd("list",ln,4)) {
    fputs("(@) (@) concat = (@1 @2)\n",stderr );
    fputs("@ dup = @1 @1\n",stderr );
    fputs("@ quote = (@1)\n",stderr );
    fputs("@ remove =\n",stderr );
    fputs("@ @ swap = @2 @1\n",stderr );
    fputs("(@) unquote = @1\n",stderr );

    list_words();
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
    return add_word(ln+3);
  }

  if (chkcmd("del",ln,3)) {
    return del_word(ln+3);
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
  fputs("  !def         define a new word\n",stderr );
  fputs("  !del         delete a sword\n",stderr );
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

char *reduce_word(vec_t stack, char *word)
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

char *reduce(vec_t stack)
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

#define MAXLINE 512
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

  dict = vecnew(char *);

  try {
 
    stack = vecnew(node_t);
    throwif(!stack,ENOMEM);

    printf("GERKU 0.0.2-beta\nType ! for available commands.\n");
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

 _dbgtrc("DICT: %d",veccount(dict));

  char **v = vec(dict);
  for (int k=0; k<veccount(dict); free(v[k++])) ;
  vecfree(dict);

  wipestack(stack);
  vecfree(stack);
  
  clear_line(line);

  return (0);
}

