/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

// DICTIONARY ******

#include "libs.h"

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

  length of the body ─╮
                      │
             ╭── n ──╮▼
  ┌──────┬─┬─┬───────┬─┬──────┬─┐
  │ word │0│n│ ┆ ┆ ┆ │l│ body │0│
  └──────┴─┴─┴───────┴─┴──────┴─┘
            ▲ ▲ 
            │ ╰─ # of occurences in the body 
            │    msb set means it is a quote
            │
            ╰─ number of arguments 

The definition:

  (@) @ X = @1 @1 Y @2 Z

Will be stored as:

  "X\0\2\2\1\xC@1 @1 Y @2 Z\0"

*/

int add_word(char *def)
{

  int nargs;

  char *dict_str;

  char *name_start, *name_end;
  char *args_start, *args_end;
  char *body_start, *body_end;
   
  uint8_t name_size;
  //uint8_t args_size;
  uint8_t body_size;

  if (!def || !*def) return 0;
  _dbgtrc("Defining %s",def);
  
  // Will surely be enough;
  dict_str = malloc(strlen(def)+16);
  
  nargs = 0;
  args_start = skp("&+s",def);
  args_end = args_start;
  while(1) {
     dict_str[nargs] = (*args_end == '@'? 0x00 : 0x80);
     args_end = skp("(&*s@&*s)\1@\2",args_end);
     if (errno) break;
     nargs++;
    _dbgtrc("ARG: %d '%02X'",nargs,(uint8_t)dict_str[nargs-1]);
     skp("&+s",args_end, &args_end);
  }
  
  if (nargs > 100) {
    printf("Error: too many arguments.\n");
    free(dict_str);
    return 0;
  }

  name_start = args_end;
  name_end = skp("&[A-Za-z_]&*[A-Za-z0-9_-]&@&*s=",name_start);
  
  if (name_end <= name_start) {
    printf("Error: Syntax error in definition.\n");
    free(dict_str);
    return 0;
  }
  
  body_start = skp("&*s=&*s",name_end);
  body_end = skp("&+.",body_start);

  int n;
  for (char *s=body_start; s < body_end; s++) {
    if (*s == '@') {
      n = atoi(s+1);
      if (n < 1 || nargs < n) {
        printf("Error: Invalid reference in definition.\n");
        free(dict_str);
        return 0;
      }
      n--;
      dict_str[n] = ((uint8_t)dict_str[n]+1);
    }
  }

 _dbgblk {
    for (int k=0; k<nargs; k++) {
      dbgtrc("ARG[%d] %02X",k,(uint8_t)dict_str[k]);
    }
  }

  while (args_end > args_start && isspace(args_end[-1]))
    args_end--;

  name_size = (uint8_t) (name_end - name_start);
  //args_size = (uint8_t) (args_end - args_start);
  body_size = (uint8_t) (body_end - body_start);

 _dbgtrc("DEF: name: '%.*s'",name_size, name_start);
 _dbgtrc("DEF: args: '%.*s' (%d)",args_size, args_start,nargs);
 _dbgtrc("DEF: body: '%.*s'",body_size, body_start);

  // move args after the name

  for (int k=nargs-1; k >=0 ; k--) { 
    dict_str[(name_size+2)+k] = dict_str[k];
  }
  dict_str[name_size+1] = (int8_t)nargs;

  memcpy(dict_str,name_start,name_size);
  dict_str[name_size]= '\0' ;

  dict_str[name_size+1+1+nargs] = (int8_t)body_size;
  memcpy(dict_str+name_size+1+1+nargs+1, body_start,body_size+1);

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

int list_words(FILE *out)
{
  char **v = vec(dict);
  char *s;
  int nargs;

  for (int k=0; k<veccount(dict); k++) {
    s=v[k];
    while (*s) s++;
    s++;
    nargs = *s++;
    for (int j=0; j<nargs; j++) {
      fprintf(out, "%s ", ( *s & 0x80 ? "(@)" : "@"));
      s++;
    }
    
    fprintf(out, "%s =",v[k]);
    s++; // len
    if (*s) fprintf(out," %s",s);
    fputc('\n',out);
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

static char *std_words[] = {
  "@ @ s=@2 @1",
  "(@) u=@1",
  "@ d=@1 @1",
  "@ r=",
  "(@) (@) c=(@1 @2)",

  "@ (@) T=@2",
  "(@) @ F=@1",

  "@ (@) K=@2",
  "(@) @ Z=@1",
  "@ (@) (@) S = (@1 @2) @1 @3",

  NULL
} ;

void init_dict()
{

  
  dict = vecnew(char *);
  throwif(!dict, ENOMEM);

  char **d = std_words;
  
  while (*d) {
    add_word(*d);
    d++;
  }
  
}
