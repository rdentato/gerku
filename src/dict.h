/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#ifndef DICT_H
#define DICT_H

void init_dict();
void free_dict();
int list_words(FILE *out);
int del_word(char *word);
int add_word(char *def);
char **search_word(char *word);

#endif