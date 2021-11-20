
#include "libs.h"

/*

Quote
Word

Number
hardw

*/





int main(int argc, char *argv[])
{
  stk_t evalstack;
  char *s;
  int l;

  evalstack = stknew();

  stkpush(evalstack,6,"pippo");
  stkpush(evalstack,6,"pluto");
  stkpush(evalstack,5,"topo");

  s = stktop(evalstack,-1);

  printf("%d:[%s]\n",stkcount(evalstack),s);

  s = stktop(evalstack,-7);
  dbgchk(s==NULL);
  stkdrop(evalstack);
  dbgchk(stkcount(evalstack) == 2);

  s = stktop(evalstack,0,&l);
  stkdrop(evalstack);
  stkdrop(evalstack);
  dbgchk(stkcount(evalstack) == 0);
  stkdrop(evalstack);
  dbgchk(stkcount(evalstack) == 0);
  
  evalstack = stkfree(evalstack);

}

