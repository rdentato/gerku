/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#include "gerku.h"

char *gerku_version="GERKU 0.0.7-beta (C) 2021 https://github.com/rdentato/gerku";

int main(int argc, char *argv[])
{
  int ret = 0;
  vec_t stack = NULL;
  int start_repl = 1;
  char *default_grk = "grk/default.grk";

  try {

    vrgver(gerku_version);
    vrgoptions(argc,argv) {
      vrgopt("-v", "Version") {
        fprintf(stderr, "%s\n", gerku_version);
      }

      vrgopt("-r", "Run mode (no REPL)") {
        start_repl = 0;
      }

      vrgopt("-d default", "Replace default combinators") {
        default_grk = vrgoptarg;
      }
    }
  
    init_dict(default_grk);
    stack = init_stack();

    //load files
    for (int i = vrgargn; i<argc; i++) {
     _dbgtrc("RUN: %s",argv[i]);
      run_file(argv[i],stack);
    }

    if (start_repl) repl(stack);    
  }
  catchall {
    thrownerr("Unexpected error");
    abort();
  }

  free_dict();
  stack = free_stack(stack);
 

  return ret;
}

