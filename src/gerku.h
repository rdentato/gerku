/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

/*                   _             
                    | |            
   __ _   ___  _ __ | | __ _   _      /\___     
  / _` | / _ \| '__|| |/ /| | | |    (    @\___ 
 | (_| ||  __/| |   |   < | |_| |   /          )
  \__, | \___||_|   |_|\_\ \__,_|  /    (_____/ 
   __/ |                          (_____/    U  
  |___/                            

  This is an interepreter for a concatenative functional language
based on combinators. Mostly on the concepts expressed in:

  "The Theory of Concatenative Combinators by Brent Kerby"
  (http://tunes.org/~iepos/joy.html#applic)
 
  Gerku is intended to explore the same space as the `mlatu` project
using a stack based semantics rather than rewriting rules. The 
aim is to verify that the two are, indeed, equivalent.

  mlatu is a project by Caden Haustein (carmysilna)
  https://github.com/mlatu-lang/mlatu


*/

#ifndef GERKU_H
#define GERKU_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "libs.h"
#include "dict.h"
#include "eval.h"
#include "repl.h"

#endif
