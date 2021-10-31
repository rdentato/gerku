/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#ifndef LIBS_H

#ifdef LIBS_MAIN
#define DBG_MAIN
#define SKP_MAIN
#define VEC_MAIN
#define TRY_MAIN
#ifdef USE_LINENOISE
#define LINENOISE_MAIN
#endif
#endif


#include "libs/dbg.h"
#include "libs/skp.h"
#include "libs/vec.h"
#include "libs/try.h"
#ifdef USE_LINENOISE
#include "libs/linenoise.h"
#endif

#endif // LIBS_H
