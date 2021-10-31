/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/
/*
# Exceptions

 Simple implementation of try/catch. try blocks can be nested.
 Exceptions are positive integers
 
  #define OUTOFMEM    1
  #define WRONGINPUT  2
  #define INTERNALERR 3
  
  try {
     ... code ...
     if (something_failed) throw(execption_num)  // must be > 0 
     some_other_func(); // you can throw exceptions from other functions too 
     ... code ...
  }  
  catch(OUTOFMEM) {
     ... code ...
  }
  catch(WRONGINPUT) {
     ... code ...
  }
  catchall {  // if not handled ...
     ... code ...
  }
                 
]]] */

#ifndef TRY_VERSION
#define TRY_VERSION 0x0100002C
// version 1.0.2rc"

#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <stdint.h>

typedef struct try_jb_s {
  jmp_buf          jb;  // Jump buffer for setjmp/longjmp
  struct try_jb_s *pv;  // Link to parent for nested try
  const char      *fn;  // Filename 
  int              ln;  // Line number
  int              ex;  // Exception number
  int16_t          ax;  // Auxiliary information
  int16_t          nn;  // Counter
} try_jb_t;

#ifdef _MSC_VER
  #define TRY_THREAD __declspec( thread )
#else
  #define TRY_THREAD __thread
#endif

extern TRY_THREAD try_jb_t *try_jmp_list;
extern char const *try_emptystring;

#define TRY_CATCH_HANDLER 0

#define try_INIT     {.pv = try_jmp_list, .nn = 0, .fn = try_emptystring, .ln = 0, .ax = 0}

#define try          for ( try_jb_t try_jb = try_INIT; \
                          (try_jb.nn-- <= 0) && (try_jmp_list = &try_jb); \
                           try_jmp_list = try_jb.pv, try_jb.nn = (try_jb.ex == 0? 2 : try_jb.nn)) \
                            if (try_jb.nn < -1) assert(TRY_CATCH_HANDLER); \
                       else if (((try_jb.ex = setjmp(try_jb.jb)) == 0)) 

#define catch(x)       else if ((try_jb.ex == (x)) && (try_jmp_list=try_jb.pv, try_jb.nn=2)) 

#define catchall       else for ( try_jmp_list=try_jb.pv; try_jb.nn < 0; try_jb.nn=2) 

void try_throw(int x, int y, char *fname, int line);
  
#define try_exp(x) x
#define try_0(x,...)   x
#define try_1(x,y,...) y
#define throw(...)  if (try_jmp_list)  \
                      try_throw(try_exp(try_0(__VA_ARGS__,1)), \
                                try_exp(try_1(__VA_ARGS__,0,0)),\
                                             __FILE__, __LINE__); \
                    else assert(TRY_CATCH_HANDLER)

#define throwagain() throw(try_jb.ex, try_jb.ax, __FILE__, __LINE__)
#define thrown()     try_jb.ex
#define thrownaux()  try_jb.ax
#define thrownfile() try_jb.fn
#define thrownline() try_jb.ln

#define thrownerr(...) (fprintf(stderr, "" __VA_ARGS__), \
                        fprintf(stderr," (%d.%d):%s:%d\n",try_jb.ex,try_jb.ax,try_jb.fn,try_jb.ln))
  
#define throwif(e,...) if (!(e)) {} else throw( __VA_ARGS__ )
#define _throwif(...) 

#ifdef TRY_MAIN
  char const *try_emptystring = ""; 
  TRY_THREAD try_jb_t *try_jmp_list=NULL;

  void try_throw(int x, int y, char *fname, int line) 
  {
    if (x > 0) {\
      try_jmp_list->fn  = fname; \
      try_jmp_list->ln  = line;
      try_jmp_list->ax  = y;
      longjmp(try_jmp_list->jb, x);
    } 
  }

#endif // TRY_MAIN

#endif // TRY_VER
