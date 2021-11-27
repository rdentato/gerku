/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#include "libs.h"
#include "dict.h"
#include "eval.h"

#include "hardwired.h"

vec_t init_stack()
{
  vec_t stack = vecnew(term_t);
  throwif(!stack,ENOMEM);
  return stack;
}

void print_stack(vec_t stack)
{
  term_t *v;

  v=vec(stack);
  fputs("|-> ",stdout);
  for (int k = 0; k<veccount(stack); k++) {
    fputs(v[k].str,stdout); fputc(' ',stdout);
  }
  putchar('\n');
}

static void wipeterm(term_t *trm)
{
  throwif(!trm,ERANGE);
  free(trm->str);
  trm->str = NULL;
  trm->len = 0;
}

void wipe_stack(vec_t stack)
{
  term_t *v;
  v=vec(stack);
  for(int k=0; k<veccount(stack); k++) {
    wipeterm(v+k);
  }
  veccount(stack,0);
}

vec_t free_stack(vec_t stack)
{
  wipe_stack(stack);
  return vecfree(stack);
}

int popterm(vec_t stack)
{
  if (veccount(stack)>0) {
    wipeterm(vectop(stack));
    vecdrop(stack);
  }
  return 0;
}

int pushterm(vec_t stack, char *start, int len)
{
  char *term;

  term = malloc(len+1 + (*start == '`'));
  throwif(!term, ENOMEM);

  memcpy(term,start,len);

  if (*start == '`') {
    *term = '(';
    term[len++] = ')';
  }

  term[len] = '\0';

  vecpush(stack,&((term_t){term, len}));
  return 0;
}


static char *reduce_word(vec_t stack, char *word)
{
  char *ret = NULL; 
  char *s;
  char arity;
  term_t *trm;
  char **w;

  w = search_word(word);

  if (w) {
    s = skp("&*.",*w);
    s++; arity = *s++ - '0';
    if (veccount(stack) < arity+1) return NULL;
    for (int i=1; i <= arity; i++) {
       trm =vectop(stack,-i);
       if (!trm || trm->str[0] != '(') return NULL;
    }

   _dbgtrc("WORD: '%s' '%s'",*w,s);
    if (*s) ret = dupstr(s);
    popterm(stack);
  }
  return ret;
}

static int is_num(char *start, int len, int *num)
{ 
  int n=0;
  char *end = start+len;
  char *tmp;
  int ret = 0;

 _dbgtrc("ISNUM: '%s' ",start);
  if (*start != '(') return 0;

  skp("&*s&+d",start+1);
 _dbgtrc("ISNUM: '%s' (err: %d)",start,errno);
  if (!errno) {
      n = atoi(start+1);
      ret=1;
  }
  else {

    skp("&+[( ]",start,&tmp);
  
   _dbgtrc("ISNUM: '%s' (err: %d)",start,errno);
    if (errno) return 0;
  
    start = tmp;
    
    skp("&+d&*[ )]",start,&tmp);
   _dbgtrc("ISNUM: '%s' (err: %d)",start,errno);
  
    if (errno) return 0;
  
    n = atoi(start);
    
    while ((start = tmp) < end ) {
      skp("&*s++&*s)",start, &tmp);
     _dbgtrc("ISNUM: '%s' (err: %d)",start,errno);
      if (errno) return 0;
      ret=1;
      n++;
    }
  }

  *num = n;
  return ret;
}

static char *reduce_incr(vec_t stack, char *word)
{
  char *ret = NULL;
  int n = -1;

  term_t *trm =vectop(stack,-1);
  if (!trm) return NULL;

  if (!is_num(trm->str,trm->len,&n))
      return NULL;

  ret = malloc(20);
  throwif(!ret,ENOMEM);

  sprintf(ret,"%d",n+1);
  popterm(stack);
  popterm(stack);

  return ret;
}

static char *reduce_decr(vec_t stack, char *word)
{
  char *ret = NULL;
  int n=0;

  term_t *trm =vectop(stack,-1);
  if (!trm) return NULL;

  if (!is_num(trm->str,trm->len,&n))
      return NULL;

  if (n>0) {
    ret = malloc(20);
    throwif(!ret,ENOMEM);
    sprintf(ret,"%d",n-1);
    popterm(stack);
  }
  popterm(stack);

  return ret;

}

static char *reduce_eq_zero(vec_t stack, char *word)
{
  char *ret = NULL;
  int n=0;

  term_t *trm =vectop(stack,-1);
  if (!trm) return NULL;

  if (!is_num(trm->str,trm->len,&n))
      return NULL;

  ret = dupstr(n==0?"$T":"$F");
  throwif(!ret,ENOMEM);

  popterm(stack);
  popterm(stack);

  return ret;

}

static char *reduce_add(vec_t stack, char *word)
{
  char *ret = NULL;
  int n1=0, n2=0;

  term_t *trm1 =vectop(stack,-1);
  if (!trm1 || !is_num(trm1->str,trm1->len,&n1))
      return NULL;

  term_t *trm2 =vectop(stack,-2);
  if (!trm2 || !is_num(trm2->str,trm2->len,&n2))
      return NULL;

  n2 += n1;
  
  ret = malloc(20);
  throwif(!ret,ENOMEM);

  sprintf(ret,"%d",n2);

  popterm(stack);
  popterm(stack);
  popterm(stack);

  return ret;

}

static char *reduce_sub(vec_t stack, char *word)
{
  char *ret = NULL;
  int n1=0, n2=0;

  term_t *trm1 =vectop(stack,-1);
  if (!trm1 || !is_num(trm1->str,trm1->len,&n1))
      return NULL;

  term_t *trm2 =vectop(stack,-2);
  if (!trm2 || !is_num(trm2->str,trm2->len,&n2))
      return NULL;

  n2 -= n1;
  if (n2<0) n2 = 0;

  ret = malloc(20);
  throwif(!ret,ENOMEM);

  sprintf(ret,"%d",n2);

  popterm(stack);
  popterm(stack);
  popterm(stack);

  return ret;

}

static char *reduce_mult(vec_t stack, char *word)
{
  char *ret = NULL;
  int n1=0, n2=0;

  term_t *trm1 =vectop(stack,-1);
  if (!trm1 || !is_num(trm1->str,trm1->len,&n1))
      return NULL;

  term_t *trm2 =vectop(stack,-2);
  if (!trm2 || !is_num(trm2->str,trm2->len,&n2))
      return NULL;

  n2 *= n1;

  ret = malloc(20);
  throwif(!ret,ENOMEM);

  sprintf(ret,"%d",n2);

  popterm(stack);
  popterm(stack);
  popterm(stack);

  return ret;
}


typedef struct {
  int16_t len;
  char *word;
  char *list;
  char *(*reduce)(vec_t stack, char *word);
} hardwired_t;

hardwired_t hardwired [] = {
  {2,"++",   "++  = *hardwired*  // increment", reduce_incr},
  {2,"--",   "--  = *hardwired*  // decrement", reduce_decr},
  {3,"=0?",  "=0? = *hardwired*  // zero?", reduce_eq_zero},
  {1,"+",    "+   = *hardwired*  // add", reduce_add},
  {1,"-",    "-   = *hardwired*  // subtract", reduce_sub},
  {1,"*",    "*   = *hardwired*  // multiply", reduce_mult},
  {0,NULL, NULL}
};

void list_hardwired(FILE *out)
{
  for (hardwired_t *hw = hardwired; hw->word; hw++) {
    fprintf(out,"        %s\n",hw->list);
  }
}

static char *reduce_hardwired(vec_t stack, char *word)
{
  int ishardwired=0;
  for (hardwired_t *hw = hardwired; hw->word; hw++) {
    ishardwired =  (strncmp(word, hw->word,hw->len) == 0)
                && ( word[hw->len] == '\0');
   _dbgtrc("HW: '%s' '%s' %d",word,hw->word,ishardwired);
    if (ishardwired) {
      return hw->reduce(stack,word);
    }
  }
  return NULL;
}

char *reduce_transparent(vec_t stack, char *word)
{
  char *ret = NULL;
  term_t *trm;
  int cnt = -1;
  int nest = 1;
  int size = 0;
  int pos;

 _dbgtrc("Closing");
  while (1) {
    trm = vectop(stack, cnt);
   _dbgtrc("@ %d (%d) '%s'",cnt,nest,trm->str);
    if (!trm) return NULL;
    if (trm->str[0] == '}') nest++;
    if (trm->str[0] == '{') nest--;
    if (nest == 0) break;
    size += trm->len;
    cnt --;
  }
 _dbgtrc("Close at:[%d] size: %d '%s'",cnt,size, trm->str);
  ret = malloc(size-3*cnt+1);
  throwif(!ret, ENOMEM);
  
  pos = 0;
  ret[pos++] = '(';
  for (int i=cnt+1; i<0; i++) {
    trm = vectop(stack, i);
    pos += sprintf(ret+pos,"%s ",trm->str);
  }
  while (pos>0 && isspace(ret[pos-1])) pos--;
  ret[pos++] = ')';
  ret[pos] = '\0';

  for (int i=0; i>=cnt; i--) {
    popterm(stack);
  }

  return ret;
}

static char *reduce(vec_t stack)
{
  term_t *trm;

  if (veccount(stack) == 0) return NULL;

  trm = vectop(stack);

  if (trm->str[0] == '{') return NULL;

  if (trm->str[0] == '}') return reduce_transparent(stack, trm->str);
  
  if (trm->str[0] == '(') return NULL;

  if (trm->str[0] == '$') return hw_combinator(stack, trm->str);

  if (isdigit(trm->str[0])) return hw_numeral(stack, trm->str);

  if (isalpha(trm->str[0]) || trm->str[0] == '_')
    return reduce_word(stack, trm->str);
    
  return reduce_hardwired(stack, trm->str);

}

typedef struct {
   char *ln;
   char *pos;
} expr_t;

static int eval_expressions(vec_t stack, vec_t expressions, int trace)
{
  char *end;
  expr_t *cur_expr;
  char *new_expr;
  int prev_depth;

  // Evaluate all the expressions in the stack;
  while (veccount(expressions) > 0) {
    cur_expr = vectop(expressions);
    new_expr = NULL;
   
    skp("&+s",cur_expr->pos,&(cur_expr->pos));

   _dbgtrc("EVAL: %s (%p)",cur_expr->pos,(void *)(cur_expr->pos));

    // evaluate current expressions till the end,
    // unless a new expression is provided!
    while (!new_expr && *(cur_expr->pos)) {

      // terms are sequence of letters/numbers or a quote
      // (a sequence of terms in parenthesis )
      end = skp(WORD_DEF "&D\3&()\4$&.\5`&+!s\6{\7}\7",cur_expr->pos);

      if (end > cur_expr->pos) { //found a term!
       _dbgtrc("PUSH: '%.*s' (%d)",(int)(end-cur_expr->pos),cur_expr->pos,(int)(end-cur_expr->pos));
        // Push the term on the evaluation stack
        pushterm(stack, cur_expr->pos, (end - cur_expr->pos));

        cur_expr->pos = skp("&+s",end);

        if (trace) print_stack(stack);
        prev_depth = veccount(stack);  // Used to avoid printing the same stack twice

        // evaluate the top of the stack. 
        // If the result is an expression, return it!
        new_expr = reduce(stack);
       _dbgtrc("REDUCED: '%s'",new_expr?new_expr:"\"\"");

        if (new_expr) {
          vecpush(expressions,&((expr_t){new_expr, new_expr}));
         _dbgtrc("PUSHED: %s (%d)",new_expr,veccount(expressions));
          break;
        }

        if (trace && (veccount(stack) != prev_depth))
          print_stack(stack);
      }
      else {
        fprintf(stderr, "ERROR: Invalid term.\n");
        fprintf(stderr, "%.*s... (%02X)\n",(int)(cur_expr->pos - cur_expr->ln + 1), cur_expr->pos,*cur_expr->pos);
        return 1;
      }
    }
    if (!new_expr) { // We reached the end of the expression. Done for this line!
      free(cur_expr->ln);
      cur_expr->ln = NULL;
      vecdrop(expressions);
    }
  }
  return 0;
}

// We'll manage the inevitable recursion with an explicit stack of expressions instead
// of recursive calls to the eval function.

int eval(vec_t stack, char *ln, int trace)
{
  int ret=0;
  char *firstln;
  vec_t exprs=NULL;

  if (!ln || !*ln) return 0;

  // Create the stack for expressions to be evaluated
  exprs = vecnew(expr_t);
  throwif(!exprs,ENOMEM);

  // Push the line got as an argument as the first line
  firstln = dupstr(ln); // Will be freed by eval_expressions
  throwif(!firstln,ENOMEM);
  vecpush(exprs,&((expr_t){firstln,firstln}));
 
  // evaluate the stack of expressions
  ret = eval_expressions(stack,exprs,trace);
  
  // Get rid of the stack of expressions
  exprs = vecfree(exprs);

  return ret;
}

