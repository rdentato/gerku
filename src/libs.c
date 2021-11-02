/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#define LIBS_MAIN
#include "libs.h"

// Don't trust strdup exists
char *dupstr(char *s)
{
  char *new_s;
  int n = strlen(s)+1;
  if ((new_s = malloc(n))) memcpy(new_s,s,n);
  return new_s;
}

// Don't trust strdup exists
char *dupnstr(char *s, int n)
{
  char *new_s;
  if ((new_s = malloc(n+1))) { memcpy(new_s,s,n); new_s[n] = '\0'; }
  return new_s;
}
