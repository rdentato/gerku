/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#include <stdio.h>
#include "libs.h"
#include "dict.h"


static int check_tail(char *skp_chk, char *frst, int var)
{
  char *s;
  s = skp(skp_chk, frst);
  if (!errno) {
    if (*s == '\0')  return 1; // no tail
    while (*s) {
      s = skp("&+![@]",s);
      if (*s == '@' && s[1] == var) 
        return 2; // var occurs in tail 
      s++;
    }
    return 3; // var does not occur in tail
  }

  return 0;
}

int select_rule(char *expr, char *frst, int var)
{
  int ret = 0;
  char skp_chk[16];
 
 _dbgtrc("RULE: expr: '%s' frst: '%s' var: '%c'",expr,frst?frst:"",var);

  //  1             {}[(@)] = zap
  //  2         {#..#}[(@)] = zap #..#
  //  3    {#..# %..%}[(@)] = (#..#) dip {%..%}[(@)]
  skp("&+s",expr, &expr);
  if (*expr == '\0') return 1;
  if (frst == NULL)  return 2;
  if (frst > expr)   return 3;

  //  4            {@}[(@)] = i
  //  5       {@ $..$}[(@)] = run {$..$}[(@)]
  //  6       {@ #..#}[(@)] = i #..#
  strcpy(skp_chk,"@x&*s");
  skp_chk[1] = var;
  ret = check_tail(skp_chk, frst, var);
  if (ret) return 3+ret;

  //  7          {(@)}[(@)] = 
  //  8     {(@) $..$}[(@)] = dup {$..$}[(@)]
  //  9     {(@) #..#}[(@)] = #..#
  strcpy(skp_chk,"(&*s@x&*s)&*s");
  skp_chk[5] = var;
  ret = check_tail(skp_chk, frst, var);
  if (ret) return 6+ret;

  // 10       {(%..%)}[(@)] = ({%..%}[(@)]) cons
  // 11  {(%..%) $..$}[(@)] = (({%..%}[(@)]) cons) sip {$..$}[(@)]
  // 12  {(%..%) #..#}[(@)] = ({%..%}[(@)]) cons #..#
  ret = check_tail("&()&*s", frst, var);
  if (ret) return 9+ret;

  return ret;
}

// #..# $..$ 
//      ^------ returns the pointer to the first 
//              term that contains the variable,
char *occurs(char *expr, int var)
{
  char *s, *end;
  char *ret = NULL;
  char *t;

  s = expr;
  s = skp("&+s",s);
  while (*s && (ret == NULL)) {

    end = skp("&()\1$&.\1{\1}\2&+![ (]\3",s);

    if (errno) break; 
    
    // found a term between s and end
    // Check if @x occurs in the term.
    t = s;
   _dbgtrc("ABST: term= '%.*s' (%c)",(int)(end-s),s,var);
    while (t<end) {
      while (t<end && *t != '@') t++;
      if (t<end && *t == '@' && t[1] == var) {
       _dbgtrc("ABST: '%.2s' in '%.*s' (%d) ?",t,(int)(end-s),s,t<end);
        ret = s;
        t = end; // Break the loops
      }
      t++;
    } 
    s = skp("&+s",end);
  }

  return ret;
}

static void abstract_var(int var, char *expr, vec_t buf) // var must be '1' for @1, '2' for @2 ...
{
   char *s = expr;
   char *frst;
   int   rule;
   
  _dbgtrc("ABST: {%s}[@%c]",expr,var);

   frst=occurs(s,var);
   rule= select_rule(s,frst,var);

  _dbgblk {
     int l1=0;
     l1 = strlen(s);
     if (frst) {
       l1 = ((int)(frst-s));
       while (l1>0 && isspace(s[l1-1])) l1--;
     }
    _dbgtrc("ABST: rule: %d '%.*s' '%s' ",rule, l1,s,frst?frst:"");
   }

   switch (rule) {
     //  1 {}[(@)] = zap
     case 1 : vecprintf(buf,"$z ");
         break;
     //  2 {#..#}[(@)] = zap #..#
     case 2 : vecprintf(buf,"$z %s", expr);
         break;

     //  3 {#..# %..%}[(@)] = (#..#) dip {%..%}[(@)]
     case 3 : s = frst;
              while (s>expr && isspace(s[-1])) s--;
              vecprintf(buf,"(%.*s) $D ", ((int)(s-expr)),expr);
              abstract_var(var,frst,buf);
         break;
     //  4 {@}[(@)] = i
     case 4:  vecprintf(buf,"$i ");
         break;

     //  5 {@ $..$}[(@)] = run {$..$}[(@)]
     case 5:  vecprintf(buf,"$r ");
              skp("@&d&*s",frst,&frst);
              abstract_var(var,frst,buf);
         break;
         
     //  6 {@ #..#}[(@)] = i #..#
     case 6:  vecprintf(buf,"$i %s ", expr+2);
         break;

     //  7 {(@)}[(@)] = 
     case 7: 
         break;

     //  8 {(@) $..$}[(@)] = dup {$..$}[(@)]
     case 8:  vecprintf(buf,"$d ");
              skp("&()&*s",frst,&frst);
              abstract_var(var,frst,buf);
         break;

     //  9 {(@) #..#}[(@)] = #..#
     case 9:  skp("&()&*s",frst,&frst);
              vecprintf(buf,"%s ", frst);
         break;

  // 10       {(%..%)}[(@)] = ({%..%}[(@)]) cons
  // 12  {(%..%) #..#}[(@)] = ({%..%}[(@)]) cons #..#
     case 10: 
     case 12: 
              skp("&()",frst,&s);
              if (s>frst) s[-1] = '\0';
              skp("&+s",s,&s);
              vecprintf(buf,"(");
              abstract_var(var,frst+1,buf);
              vecprintf(buf,") $c %s ",s);
         break;

  // 11  {(%..%) $..$}[(@)] = ({%..%}[(@)]) cosp {$..$}[(@)]
     case 11: 
              skp("&()",frst,&s);
              if (s>frst) s[-1] = '\0';
              skp("&+s",s,&s);
              vecprintf(buf,"(");
              abstract_var(var,frst+1,buf);
              vecprintf(buf,") $C ");
              abstract_var(var,s,buf);
         break;

   }

}

char *abstract(char *expr, int exprlen, int var, char *name, int namelen)
{
  char *ret = NULL;
  vec_t buf = NULL;
  int from, to;
  
  char *dup_expr;
  
  dup_expr = (exprlen <=0)? dupstr(expr) : dupnstr(expr,exprlen);
  throwif(!dup_expr,EINVAL);

  if (var <= 0) {

    from = '1'; to ='0'-var;
    for( char *s=dup_expr; *s ; s++) {
      if (*s == '@' && isdigit(s[1]) && s[1]>to)
        to = s[1];
    }
  }
  else {
    from = var;
    to = var;
  }


  for (var = from; var <= to; var++) {
     buf = vecnew(char);
     vecputs(buf, "");
    
     abstract_var(var, dup_expr, buf);
     free(dup_expr);

     dup_expr = (char *) vectoarray(buf);
  }

  throwif(!dup_expr,EINVAL);

  char *s = dup_expr;
  while (*s) s++;
  while (s > dup_expr && isspace(s[-1])) *--s = '\0';

 _dbgtrc("ABST: pre-name: '%s'",dup_expr);
  if (name) {
     buf = vecnew(char);
     vecprintf(buf,"%.*s__%s",namelen,name,dup_expr);
    _dbgtrc("VLN: %d",veccount(buf));
     dup_expr = (char *) vectoarray(buf);
     dup_expr[namelen] = '\0';
     dup_expr[namelen+1] = to;
  }
  
  // printf("     > !def %s = %s\n",dup_expr, dup_expr+namelen+1);

  ret = dup_expr;
  return ret;
}
