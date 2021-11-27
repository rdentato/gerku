

#include "hardwired.h"

//    $i  i    = @1
char *hw_comb_i(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-1);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  ret = malloc(trm1->len-1);
  throwif(!ret, ENOMEM);

  memcpy(ret,trm1->str+1,trm1->len-2);
  ret[trm1->len-2] = '\0';

  popterm(stack);
  popterm(stack); // remove comb
  return ret;
}

//    $z  zap  = 
char *hw_comb_z(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-1);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  popterm(stack);
  popterm(stack); // remove comb
  return ret;
}

//    $r  run  = @1 (@1)
char *hw_comb_r(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-1);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  ret = malloc(2 * trm1->len + 4);
  throwif(!ret, ENOMEM);

  sprintf(ret,"%.*s %s",trm1->len-2,trm1->str+1,trm1->str);
  
  popterm(stack);
  popterm(stack); // remove comb
  return ret;
}

//    $d  dup  = (@1) (@1)
char *hw_comb_d(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-1);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  ret = malloc(2 * trm1->len + 4);
  throwif(!ret, ENOMEM);

  sprintf(ret,"%s %s",trm1->str,trm1->str);
  
  popterm(stack);
  popterm(stack); // remove comb
  return ret;
}

//    $c  cons = ((@1) @2)
char *hw_comb_c(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-2);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  term_t *trm2 =vectop(stack,-1);
  if (!trm2 || trm2->str[0] != '(')
      return NULL;

  ret = malloc(trm1->len + trm2->len +8);
  throwif(!ret,ENOMEM);

  sprintf(ret,"(%s %.*s)",trm1->str,trm2->len-2,trm2->str+1);

  popterm(stack);
  popterm(stack);
  popterm(stack);
  
  return ret;
}

//    $C  cosp = ((@1) @2) (@1)
char *hw_comb_C(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-2);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  term_t *trm2 =vectop(stack,-1);
  if (!trm2 || trm2->str[0] != '(')
      return NULL;

  ret = malloc(2*trm1->len + trm2->len +8);
  throwif(!ret,ENOMEM);

  sprintf(ret,"(%s %.*s) %s",trm1->str,trm2->len-2,trm2->str+1,trm1->str);

  popterm(stack);
  popterm(stack);
  popterm(stack);
  
  return ret;
}

//    $D  dip  = @2 (@1)
char *hw_comb_D(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-2);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  term_t *trm2 =vectop(stack,-1);
  if (!trm2 || trm2->str[0] != '(')
      return NULL;

  ret = malloc(trm1->len + trm2->len +8);
  throwif(!ret,ENOMEM);

  sprintf(ret,"%.*s %s",trm2->len-2,trm2->str+1,trm1->str);
  
  popterm(stack);
  popterm(stack);
  popterm(stack);
  
  return ret;
}

//    $S  sip  = (@1) @2 (@1)
char *hw_comb_S(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-2);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  term_t *trm2 =vectop(stack,-1);
  if (!trm2 || trm2->str[0] != '(')
      return NULL;

  ret = malloc(2*trm1->len + trm2->len +8);
  throwif(!ret,ENOMEM);

  sprintf(ret,"%s %.*s %s",trm1->str,trm2->len-2,trm2->str+1,trm1->str);

  popterm(stack);
  popterm(stack);
  popterm(stack);
  
  return ret;
}

//    $T  true = (@2)
char *hw_comb_T(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-2);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  term_t *trm2 =vectop(stack,-1);
  if (!trm2 || trm2->str[0] != '(')
      return NULL;

  ret = trm2->str;
  trm2->str = NULL;

  char *s=ret;
  while (s[1]) {
    s[0] = s[1];
    s++;
  }
  if (s>ret && s[-1] == ')') s[-1] = '\0';
  popterm(stack);
  popterm(stack);
  popterm(stack);
  
  return ret;
}

//    $F  false/2 = (@1)
char *hw_comb_F(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-2);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  term_t *trm2 =vectop(stack,-1);
  if (!trm2 || trm2->str[0] != '(')
      return NULL;

  ret = trm1->str;
  trm1->str = NULL;

  char *s=ret;
  while (s[1]) {
    s[0] = s[1];
    s++;
  }
  if (s>ret && s[-1] == ')') s[-1] = '\0';
  popterm(stack);
  popterm(stack);
  popterm(stack);
  
  return ret;
}

char *hw_define(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-2);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  term_t *trm2 =vectop(stack,-1);
  if (!trm2 || trm2->str[0] != '(')
      return NULL;

  def_word(trm2->str,trm1->str);

  popterm(stack);
  popterm(stack);
  popterm(stack);
  
  return ret;
}

//    $N  next = ((@1) @2) (@2)
char *hw_comb_nxt(vec_t stack, char *word)
{
  char *ret = NULL;

  term_t *trm1 =vectop(stack,-2);
  if (!trm1 || trm1->str[0] != '(')
      return NULL;

  term_t *trm2 =vectop(stack,-1);
  if (!trm2 || trm2->str[0] != '(')
      return NULL;

  ret = malloc(trm1->len + 2*trm2->len +8);
  throwif(!ret,ENOMEM);

  sprintf(ret,"(%s %.*s) %s",trm1->str,trm2->len-2,trm2->str+1,trm2->str);

  popterm(stack);
  popterm(stack);
  popterm(stack);
  
  return ret;
}

char *hw_closepar(vec_t stack, char *word)
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
    if (trm->str[0] == '$') {
      if (trm->str[1] == '}') nest++;
      if (trm->str[1] == '{') nest--;
    }
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


char *hw_combinator(vec_t stack, char *word)
{
  switch (word[1]) {
    case 'i' : return hw_comb_i(stack, word);  // i
    case 'z' : return hw_comb_z(stack, word);  // zap
    case 'r' : return hw_comb_r(stack, word);  // run
    case 'd' : return hw_comb_d(stack, word);  // dup
    case 'c' : return hw_comb_c(stack, word);  // cons
    case 'C' : return hw_comb_C(stack, word);  // cosp
    case 'D' : return hw_comb_D(stack, word);  // dip
    case 'S' : return hw_comb_S(stack, word);  // sip

    case 'n' : return hw_comb_nxt(stack, word);  // sip

    case 'T' : return hw_comb_T(stack, word);  // true
    case 'F' : return hw_comb_F(stack, word);  // false

    case '=' : return hw_define(stack, word);

    case '}' : return hw_closepar(stack, word);
  }
  return NULL;
}

char *hw_numeral(vec_t stack, char *word)
{
  char *ret = NULL;

  int32_t size = 0;
  // Check for args to be quotes
  term_t *trm1 =vectop(stack,-1);
  if (!trm1 || trm1->str[0] != '(') return NULL;

  term_t *trm2 =vectop(stack,-2);
  if (!trm2 || trm2->str[0] != '(') return NULL;

  int n = atoi(word);

  size = n + (trm2->len) + n * (trm1->len-1+1)+4;

  ret = malloc(size);
  throwif(!ret,ENOMEM);
    
  char *s;
  s = ret;
  for (int i=0; i<n-1; i++) { 
    *s++ = '(';
  }

  memcpy(s,trm2->str,trm2->len);
  s += trm2->len;

  for (int i=0; i<n; i++) {
    *s++ = ' ';
    memcpy(s,trm1->str+1,trm1->len-1);
    s += trm1->len-1;
  }
  if (s>ret &&s[-1] == ')') s--;
  *s = '\0'; 
  throwif(s >= ret+size,ERANGE);

  popterm(stack);
  popterm(stack);
  popterm(stack);

  return ret;            
}

