/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#include "gerku.h"


int main(int argc, char *argv[])
{
  int ret = 0;
  vec_t stack = NULL;

  try {
    init_dict();
    stack = init_stack();
    ret = repl(stack);    
  }
  catchall {
    thrownerr("Unexpected error");
    abort();
  }

  free_dict();
  stack = free_stack(stack);
 

  return ret;
}

