#ifndef HARDWIRED_H
#define HARDWIRED_H

#include "libs.h"
#include "dict.h"
#include "abstract.h"
#include "eval.h"

char *hw_combinator(vec_t stack, char *word);
char *hw_numeral(vec_t stack, char *word);

#endif