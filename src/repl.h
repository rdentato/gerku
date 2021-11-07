/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#ifndef REPL_H
#define REPL_H

#include "libs.h"

int run_file(char *filename, vec_t stack);
int repl(vec_t stack);

#endif