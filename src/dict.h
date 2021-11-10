/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#ifndef DICT_H
#define DICT_H

void init_dict(char *file);
void free_dict();
int list_words(FILE *out,int def);
int del_word(char *word);
int add_word(char *def);
char **search_word(char *word);
int load_defs(char *filename);
int save_defs(char *filename);
int del_dict();

#define WORD_DEF "&[A-Za-z_]&*[A-Za-z0-9_-]&?[?!]"

#endif