/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

// DICTIONARY ******

#include "libs.h"
#include "dict.h"
#include "abstract.h"

static vec_t dict = NULL;

char **search_word(char *word)
{
  char **v=vec(dict);
  for (int k=0; k<veccount(dict); k++) {
    if (strcmp(v[k],word) == 0) return v+k;
  }
  return NULL;
}

/* 
  New words will be stored in the dictionary as a single
  string with the following structure:

  ┌──────┬─┬──────┬─┐
  │ word │0│ body │0│
  └──────┴─┴──────┴─┘
*/

int def_word(char *name, char *body)
{
  int var =0;
  char *dict_str;

  char *name_start, *name_end;
  char *body_start, *body_end;
  
  name_start = skp("&+[( ]",name);
  name_end = skp("&I&?[?!]",name_start);

  if (*name_end == '/') {
    var = -atoi(name_end+1);
  }
  body_start= skp("&+s",body);
  if (*body_start == '(') body_start++;
  body_end= skp("&+.",body);
  while (body_end>body_start && isspace(body_end[-1])) --body_end;
  
  if (body_end>body_start && body_end[-1] == ')') body_end--;

  _dbgtrc("[%.*s] '%.*s'",(int)(name_end - name_start),name_start,(int)(body_end - body_start),body_start);
  if (name_end <= name_start || body_end <= body_start) {
    fprintf(stderr, "Error: Syntax error in definition.\n");
    fprintf(stderr, "%s\n",body_start);
    return 0;
  }

  dict_str = abstract(body_start,body_end - body_start,var,name_start, name_end - name_start);

  char **w = search_word(dict_str);

  if (w) { // Word already exists -> replace it!
    free(*w);
    *w = dict_str;
  }
  else { // New word -> add to the dictionary
    vecpush(dict, &dict_str);
  }

  return 0;
}

int add_word(char *def)
{
  int var =0;
  char *dict_str;

  char *name_start, *name_end;
  

  if (!def || !*def) return 0;
 _dbgtrc("Defining %s",def);
  
  name_end = def;
  name_start = skp("&*s",def);
  name_end = skp("&I&?[?!]",name_start);

  if (*name_end == '/') {
    var = -atoi(name_end+1);
  }

 _dbgtrc("NAME: '%.*s'",(int)(name_end - name_start),name_start);

  skp("&>=&*s",name_end,&def);
  
 _dbgtrc("DEF: %d '%s'",var,def);

  if (name_end <= name_start || def<=name_end) {
    fprintf(stderr, "Error: Syntax error in definition.\n");
    fprintf(stderr, "%s\n",def);
    return 0;
  }
  
  dict_str = abstract(def,0,var,name_start, name_end - name_start);

  char **w = search_word(dict_str);

  if (w) { // Word already exists -> replace it!
    free(*w);
    *w = dict_str;
  }
  else { // New word -> add to the dictionary
    vecpush(dict, &dict_str);
  }

  return 0;
}

int del_word(char *word)
{
  char **w;
  char **x;

  while (isspace(*word)) word++;
  w = search_word(word);
  if (w) { 
    // swap the defintion with the
    // last one and remove the last one.
    free(*w); *w = NULL;
    x = vectop(dict);
    *w = *x;
    vecdrop(dict);
  }
  return 0;
}

int del_dict()
{
  char **w;
  int k;

  k = veccount(dict);
  if (veccount(dict) > 0) {
    w = vec(dict);
    while (k-- > 0) {
      free(*w++);
    }
  }
  vecclean(dict);
  return 0;
}

int list_words(FILE *out,int def)
{
  char **v = vec(dict);
  char *s;

  for (int k=0; k<veccount(dict); k++) {
    s=v[k];
    while (*s) s++;
    s++;

    if (def) fprintf(out,"!def ");
  
    fprintf(out, "%s = %s\n",v[k],s);
  }
  return 0;
}

void free_dict()
{
 _dbgtrc("DICTfree: %d",veccount(dict));

  char **v = vec(dict);
  for (int k=0; k<veccount(dict); free(v[k++])) ;
  dict = vecfree(dict);
}

#define MAXLEN 1024

int load_defs(char *filename)
{
  int ret =1;
  FILE *f = NULL;
  char *def;
  
  char *buf = NULL;

  f=fopen(filename,"r");
  if (f) {
    ret = 0;
    buf = malloc(MAXLEN);
    throwif(!buf,ENOMEM);

    while (fgets(buf,MAXLEN,f)) {
      def = skp("&+s",buf);
     _dbgtrc("LOADING: %s",def);
      if (strncmp("!def ",def,5) == 0) {
       _dbgtrc("Loading def for: `%s`",def+5);
        add_word(def+5);
      }
    }
    free(buf);
    fclose(f);
  }

  return ret;
}

int save_defs(char *filename)
{
  int ret = 1;
  FILE *f = NULL;

  if ((f = fopen(filename,"w"))) {
    list_words(f,1);
    ret = 0;
    fclose(f);
  }
    
  return ret;
}

// No longer used

#if 0
static char *std_words[] = {
      "(@) (@) W = (@1) (@1) @2",
      "(@) (@) K = @2",
      "(@) (@) O = @1",
  "(@) (@) (@) B = ((@1) @2) @3",
  "(@) (@) (@) C = (@2) (@1) @3",
          "(@) I = @1",
          "(@) D = (@1) (@1)",
      "(@) (@) E = (@2) (@1)",
      "(@) (@) J = (@1 @2)",
          "(@) Q = ((@1))",
  "(@) (@) (@) S = ((@1) @2) (@1) @3",
  NULL
} ;
#endif 

void init_dict(char *file)
{
  dict = vecnew(char *);
  throwif(!dict, ENOMEM);

  if (!file || !*file) return;
#if 0
  char **d = std_words;
  
  while (*d) {
    add_word(*d);
    d++;
  }
#endif

  if (load_defs(file))
    fprintf(stderr,"Unable to load default");

}

