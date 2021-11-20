
/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#include "libs.h"
#include "dict.h"
#include "eval.h"
#include "abstract.h"


#define QUIT 255
#define WIPE 254

static int trace = 0;
static int wipe = 1;

#define chkcmd(s,l,n) ((strncmp(s,l,n) == 0) && ((l[n] == '\0') || isspace(l[n])))
static int command(char *ln)
{
  if (chkcmd("list",ln,4)) {
    list_words(stdout,0);
    list_hardwired(stdout);
    return 0;
  }

  if (chkcmd("trace",ln,5)) {
    trace = !trace;
    fprintf(stderr,"Trace is %s\n",&("OFF\0ON"[(trace*4)]));
    return 0;
  }

  if (chkcmd("print",ln,5)) {
    return 0;
  }

  if (chkcmd("def",ln,3)) {
  
    return add_word(ln+3);
  }

  if (chkcmd("del",ln,3)) {
    skp("&+s",ln+3,&ln);
    if (strncmp(ln,"!all",4) == 0)
      return del_dict();
    else
      return del_word(ln);
  }

  if (chkcmd("wipe",ln,4)) {
    skp("&+![a]",ln,&ln);
    if (strncmp(ln,"auto",4) == 0) {
      wipe = !wipe;
      fprintf(stderr,"Wipe is %s\n",&("OFF\0ON"[(wipe*4)]));
    }
    return WIPE;
  }

  if (chkcmd("abstract",ln,8)) {
    char *a;
    skp("&+s",ln+8,&ln);
    a = abstract(ln,0,0,NULL,0);
    free(a);
    return 0;
  }

  if (chkcmd("load",ln,4)) {
    skp("&+s",ln+4,&ln);
    if (load_defs(ln)) 
      fprintf(stderr,"Unable to load definitions.\n");
    return 0;
  }

  if (chkcmd("save",ln,4)) {
    skp("&+s",ln+4,&ln);
    if (save_defs(ln)) 
      fprintf(stderr,"Unable to save definitions.\n");
    return 0;
  }

  if (chkcmd("quit",ln,4)) {
    return QUIT;
  }

  if (!chkcmd("help",ln,4)) {
    fputs("Available commands:\n",stderr);
  }

  fputs("  !help              this help\n",stderr );
  fputs("  !list              list of defined words\n",stderr );
  fputs("  !load file         load definitions from file\n",stderr );
  fputs("  !save file         save definitions to file\n",stderr );
  fputs("  !print             print current stack\n",stderr );
  fputs("  !trace             toggle reduction tracing\n",stderr );
  fputs("  !quit              exit the repl\n",stderr );
  fputs("  !def ...           define a new word\n",stderr );
  fputs("  !del word | !all   delete a word [all words!]\n",stderr );
  fputs("  !wipe [auto]       wipe the stack [and toggles autowipe]\n",stderr );

  return 0;
}

#define MAXLEN 1024

#ifdef USE_LINENOISE
  #define get_line(l) (l = linenoise("gerku> "))
  char *buf=NULL;
  #define clear_line(l) (l? free(l) : 0, l = NULL)
              
  #define load_history() do { linenoiseHistorySetMaxLen(50);\
                             linenoiseHistoryLoad(".history.grk"); \
                        } while(0)

  #define add_history(line) do { linenoiseHistoryAdd(line); \
                                linenoiseHistorySave(".history.grk"); \
                              }  while(0)
#else 
  char buf[MAXLEN];
  #define get_line(l) (fprintf(stdout,"gerku> "),fgets(l,MAXLEN,stdin))
  #define clear_line(l) (*l='\0');
  #define load_history()
  #define add_history(line)
#endif


int run_file(char *filename, vec_t stack)
{
  int ret =1;
  FILE *f = NULL;
  char *ln;
  char *line = NULL;

  int trc = trace;
  int wpe = wipe;

  f=fopen(filename,"r");
  if (f) {
    ret = 0;
    wipe = 0;
    trace = 0;

    line = malloc(MAXLEN);
    throwif(!line,ENOMEM);

   _dbgtrc("Running %s",filename);

    while (fgets(line,MAXLEN,f)) {
      ln = line;
      skp("&+s",ln,&ln);
     _dbgtrc("line: '%s'",ln);
      if (*ln && *ln != '#') {
        ret = (*ln == '!')? command(ln+1) : eval(stack,ln,trace);
      }
      if (ret == WIPE || wipe) wipe_stack(stack);
    }

    free(line);
    fclose(f);
  }

  print_stack(stack);
  trace = trc;
  wipe = wpe;
  return ret;
}


extern char *gerku_version;
int repl(vec_t stack)
{
  int ret = 0;
  char *ln;
  char *line = buf;

  printf("%s\nType ! for available commands.\n",gerku_version);
  print_stack(stack);
 _dbgtrc("DBG TRACE");
  load_history();
  
  while((ret != QUIT) && (get_line(line) != NULL)) {
    add_history(line);

    ln = line;
    skp("&+s",ln,&ln);

    ret = (*ln == '!')? command(ln+1) : eval(stack,ln,trace);

    clear_line(line);
    if (ret == WIPE) wipe_stack(stack);
    print_stack(stack);
    if (wipe) wipe_stack(stack);
  }
  return ret;
}

