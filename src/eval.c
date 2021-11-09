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
} term_t;

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

static int popterm(vec_t stack)
{
  if (veccount(stack)>0) {
    wipeterm(vectop(stack));
    vecdrop(stack);
  }
  return 0;
}

static int pushterm(vec_t stack, char *start, int len)
{
  char *term;
  
  term = malloc(len+1);
  throwif(!term, ENOMEM);

  memcpy(term,start,len);
  term[len] = '\0';
  vecpush(stack,&((term_t){term, len}));

  return 0;
}


static char *reduce_word(vec_t stack, char *word)
{
  char *ret = NULL; 
 _dbgtrc("REDUCE WORD: '%s'",word);

  char **w = search_word(word);
  if (w) {
    int32_t size = 0;
    term_t *trm;
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
    trm = vectop(stack); // word
    term_t *trm1 = trm - nargs;
    // Check args type (quote/term)
    for (int k=0; k< nargs; k++, s++, trm1++) {
      
      if ((*s & 0x80) && (trm1->str[0] != '('))
        return NULL;

      size += trm1->len * (*s & 0x7F);
    }

    // Build expressions
    size += *s++;
   _dbgtrc("size=%d",size);
    if (*s) { // if the body is empty, no need of additional reduce
      ret = malloc(size);
      *ret = '\0';
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
  
          memcpy(t,trm[n].str+l ,trm[n].len - (2*l));
  
          t += trm[n].len - (2*l);
  
          //*t++ =' ';
        }
        else *t++ = *s;
        *t = '\0';
        s++;
      }
    }
   _dbgtrc("expr: '%s'",ret);
    // pop
    for (int k=0; k<=nargs; k++)
      popterm(stack);
  }

  return ret;
}


static char *reduce(vec_t stack)
{
  char *ret = NULL;
  term_t *trm;

  if (veccount(stack) == 0) return NULL;

  trm = vectop(stack);

  if (trm->str[0] == '(') return NULL;

  ret = reduce_word(stack, trm->str);

  return ret;
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
  int alt;

  // Evaluate all the expressions in the stack;
  while (veccount(expressions) > 0) {
    cur_expr = vectop(expressions);
    new_expr = NULL;
   
    skp("&+s",cur_expr->pos,&(cur_expr->pos));

   _dbgtrc("EVAL: %s (%p)",cur_expr->pos,(void *)(cur_expr->pos));

    // evaluate current expressions till the end,
    // unless a new expression is provided!
    while (*(cur_expr->pos) && !new_expr) {

      // terms are sequence of letters/numbers or a quote
      // (a sequence of terms in parenthesis )
      end = skp("&[A-Za-z_]&*[A-Za-z0-9_-]&?[?!]\1&B\2",cur_expr->pos, NULL, &alt);
      // Only accept '(' as parenthesis (not '[' or '{')!
      if ((alt == '\2') && (*(cur_expr->pos) != '(')) end = NULL;

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
        }
        else {
          if (trace && (veccount(stack) != prev_depth))
            print_stack(stack);
        }
        
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

