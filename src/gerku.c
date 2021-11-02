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

static int trace = 0;

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

    list_words(stdout);
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


  try {

    init_dict();
    stack = init_stack();

    printf("GERKU 0.0.2-beta\nType ! for available commands.\n");
    print_stack(stack);

    while((ret != QUIT) && (get_line(line) != NULL)) {
      ln = line;
      skp("&+s",ln,&ln);

      ret = (*ln == '!')? command(ln+1) : eval(stack,ln,trace);

      clear_line(line);
      if (ret == WIPE) wipe_stack(stack);
      print_stack(stack);
    }
    
  }
  catchall {
    thrownerr("Unexpected error");
    abort();
  }

  free_dict();
  stack = free_stack(stack);
 
  clear_line(line);

  return (0);
}

