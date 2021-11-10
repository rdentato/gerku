/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#ifndef EVAL_H
#define EVAL_H

#include "libs.h"
#include "dict.h"

vec_t init_stack();
vec_t free_stack(vec_t stack);
void print_stack(vec_t stack);
void wipe_stack(vec_t stack);
int eval(vec_t stack, char *ln, int trace);

#endif
