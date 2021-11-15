/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

#ifndef SKP_VER
#define SKP_VER 0x0002001C
#define SKP_VER_STR "0.2.1rc"

#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <stdbool.h>

#define skp_exp(x) x
#define skp_0(x,...)     x
#define skp_1(x,y,...)   y
#define skp_2(x,y,z,...) z

char *skp_(char *pat, char *src,char **end, int *alt);

#define skp(pat,...) skp_(pat, \
                          skp_exp(skp_0(__VA_ARGS__,NULL)), \
                          skp_exp(skp_1(__VA_ARGS__,NULL,NULL)), \
                          skp_exp(skp_2(__VA_ARGS__,NULL,NULL,NULL)) )

#define skp_join(x,y) x ## y
#define skp_cat(x,y) skp_join(x,y)
#define skp_lbl skp_cat(skp_lbl_,__LINE__)


#define skpwhile(s_)  for(int skp_flg = 1; skp_flg; ) { \
                       char *skpstart; \
                       char *skpend; \
                       char *skp_last; \
                       int skp_matched; \
                       if (skp_flg == 1) { skpend = s_; skp_last = NULL; skp_flg = 2; }\
                       skpstart = skpend; \
                      _dbgtrc("END: %c",*skpend); \
                       skp_matched = 0; \
                       if (!*skpstart || (skpstart == skp_last)) break; \
                       skp_last = skpstart; \
                       
#define skpcase(e_)   } \
                      if (!skp_matched) skp(e_,skpstart,&skpend); \
                      if (!skp_matched && (skp_matched = !errno)) { \
                      goto skp_lbl; skp_lbl

#define skpdefault    } \
                      if (skp_matched) continue; \
                      goto skp_lbl; skp_lbl




#ifdef SKP_MAIN

#include "dbg.h"

/*
   a ASCII alphabetic char
   l ASCII lower case
   u ASCII upper case
   d decimal digit
   x hex digit
   w white space (includes some Unicode spaces)
   s white space and vertical spaces (e.g. LF)
   c control
   n newline
   
   . any (UTF-8 or ISO character)
 
   Q Quoted string with '\' as escape
   B Balanced sequence of parenthesis
   I Identifier ([_A-Za-z][_0-9A-Za-z]*)
   N Any number (the longest among the three below )
   D integer decimal number (possibly signed)
   F floating point number (possibly with sign and exponent)
   X hex number (possibly with leading 0x)
  
   C case sensitive (ASCII) comparison
   U utf-8 encoding (or ASCII/ISO-8859)

   * zero or more match
   ? zero or one match
   + one or more match

   ! negate test

   @ set goal

   [...] set
  
  &.  (any non \0 character)
  &!. (eol)

  &>  skip to the start of pattern

  &&

*/

static uint32_t skp_next(char *s,char **end,int iso)
{
  uint32_t c = 0;

  if (*s) {
    c = *s++;
    if (!iso) {
      while ((*s & 0xC0) == 0x80) {
         c = (c << 8) | *s++;
      }
    }
    if (c == 0x0D && *s == 0x0A) {
      c = 0x0D0A; s++;
    }
  }

  if (end) *end = s;
  return c;
}

static int chr_cmp(uint32_t a, uint32_t b, int fold)
{
  if (fold && a <= 0xFF && b <= 0xFF) {
    a = tolower(a);
    b = tolower(b);
  }
  return (a == b);
}

static int is_blank(uint32_t c)
{
  return (c == 0x20) || (c == 0x09)
      || (c == 0xA0) || (c == 0xC2A0)
      || (c == 0xE19A80)
      || ((0xE28080 <= c) && (c <= 0xE2808A))
      || (c == 0xE280AF)
      || (c == 0xE2819F)
      || (c == 0xE38080)
      ;
}

static int is_break(uint32_t c)
{
  return (c == 0x0A)      // U+000A LF line feed           
      || (c == 0x0C)      // U+000C FF form feed          
      || (c == 0x0D)      // U+000D CR carriage return     
      || (c == 0x85)      // U+0085 NEL next line   ISO-8859-15      
      || (c == 0x0D0A)    // CRLF (not a real UTF-8 CODEPOINT!!!)
      || (c == 0xC285)    // U+0085 NEL next line         
      || (c == 0xE280A8)  // U+2028 LS line separator     
      || (c == 0xE280A9)  // U+2029 PS paragraph separator
      ;
}

static int is_space(uint32_t c)
{ return is_blank(c) || is_break(c); }

static int is_digit(uint32_t c)
{ return ('0' <= c && c <= '9'); }

static int is_xdigit(uint32_t c)
{
  return ('0' <= c && c <= '9')
      || ('A' <= c && c <= 'F')
      || ('a' <= c && c <= 'f');
}

static int is_upper(uint32_t c)
{ return ('A' <= c && c <= 'Z'); }

static int is_lower(uint32_t c)
{ return ('a' <= c && c <= 'z'); }

static int is_alpha(uint32_t c)
{ return ('A' <= c && c <= 'Z')
      || ('a' <= c && c <= 'z'); }

static int is_alnum(uint32_t c)
{ return (is_alpha(c) || is_digit(c)); }

static int is_ctrl(uint32_t c)
{ return (c < 0x20)
      || (0xC280 <= c && c <= 0xC2A0)
      || (0x80 <= c && c <= 0xA0);
}

static int is_oneof(uint32_t ch, char *set, int iso)
{
  uint32_t p_ch,q_ch;
  char *s;
  if (ch == '\0') return 0;
 _dbgtrc("set: [%s] chr: %c",set,ch);
  p_ch = skp_next(set,&s,iso);
  
  if (p_ch == ']' && ch == ']') return 1;

  while (p_ch != ']') {
    if (p_ch == ch) return 1;
    q_ch = p_ch;
    p_ch = skp_next(s,&s,iso);
    if ((p_ch == '-') && (*s != ']')) {
      p_ch = skp_next(s,&s,iso);
      if ((q_ch < ch) && (ch <= p_ch)) return 1;
      p_ch = skp_next(s,&s,iso);
    }
  }
  p_ch = skp_next(s,&s,iso);
  return 0;
}

static uint32_t get_close(uint32_t open)
{
   switch(open) {
     case '(': return ')';
     case '[': return ']';
     case '{': return '}';
     case '<': return '>';
   }
   return 0;
}

#define MATCHED_FAIL    0 
#define MATCHED         1
#define MATCHED_GOAL    2
#define MATCHED_GOALNOT 3

static int match(char *pat, char *src, char **pat_end, char **src_end)
{
  uint32_t p_chr, s_chr;
  char *p_end, *s_end;
  int ret = 0;
  uint32_t match_min = 1;
  uint32_t match_max = 1;
  uint32_t match_cnt = 0;
  uint32_t match_not = 0;
  int fold = false;
  int iso = false;
  int intnumber  = false;
  char *s_tmp = src;
  
  s_end = src;
  s_chr = skp_next(s_end, &s_tmp,iso);

  if (*pat == '&') {
    pat++;
    
    if (*pat == '*') { match_min = 0;  match_max = UINT32_MAX; pat++; } 
    else if (*pat == '+') { match_max = UINT32_MAX; pat++; } 
    else if (*pat == '?') { match_min = 0; pat++; }
    
    if (*pat == '!') { match_not = 1; pat++; }
   _dbgtrc("min: %u max: %u not: %u",match_min, match_max, match_not);

    #define W(x)  \
      do { \
       _dbgtrc("matchedW: '%s' schr: %X test: %d (%s)",s_end,s_chr,(x),#x); \
        for (match_cnt = 0; \
             (match_cnt < match_max) && s_chr && (!!(x) != match_not); \
             match_cnt++) { \
          s_end = s_tmp; s_chr = skp_next(s_end,&s_tmp,iso); \
        } \
        ret = (match_cnt >= match_min); \
       _dbgtrc("cnt: %d ret: %d s: %c end: %c",match_cnt,ret,*s_tmp, *s_end); \
      } while (0)

    #define get_next_s_chr() do {s_end = s_tmp; s_chr = *s_end ; s_tmp++;} while(0)
    
    intnumber = false;
    
    switch (*pat++) {
      case '&' : ret = (s_chr == '&') ;  break;

      case '.' : if (match_not) ret = (s_chr == 0);
                 else W(s_chr != 0);
                 break;

      case 'd' : W(is_digit(s_chr)); break;
      case 'x' : W(is_xdigit(s_chr)); break;
      case 'a' : W(is_alpha(s_chr)); break;
      case 'u' : W(is_upper(s_chr)); break;
      case 'l' : W(is_lower(s_chr)); break;
      case 's' : W(is_space(s_chr)); break;
      case 'w' : W(is_blank(s_chr)); break;
      case 'n' : W(is_break(s_chr)); break;
      case 'c' : W(is_ctrl(s_chr)); break;
      
      case '@' : ret = match_not? MATCHED_GOALNOT : MATCHED_GOAL;
                 break;

      case '[' : W(is_oneof(s_chr,pat,iso));
                 while (*pat && *pat != ']') pat++;
                 if (*pat && pat[1]==']') pat++;
                 pat++;
                 break;

      case 'C' : fold = !match_not; ret = MATCHED;
                 break;

      case 'U' : iso = match_not; ret = MATCHED;
                 break;

      case 'I' : // Identifier
                if (is_alpha(s_chr) || (s_chr == '_')) {
                  do {
                    get_next_s_chr();
                  } while (is_alnum(s_chr) || (s_chr == '_'));
                  ret = MATCHED;
                } 
                break;

      case '(' : if (*pat != ')' || s_chr != '(') break;
                 pat++;

      case 'B' : // Balanced parenthesis
                 {
                   uint32_t open;
                   uint32_t close;
                   int32_t count;
                   open = s_chr;
                   close = get_close(open);
                   if (close != '\0') {
                     count=1;
                     while (s_chr && count > 0) {
                       get_next_s_chr();
                       if (s_chr == open)  count++;
                       if (s_chr == close) count--;
                     }
                     if (count == 0) {
                       get_next_s_chr();
                       ret = MATCHED;
                     }
                   }
                 }
                 break;

      case 'X' : // hex number
                if (   (s_chr == '0')
                    && (s_end[1] == 'x' || s_end[1] == 'X') 
                    && is_xdigit(s_end[2])
                   ) {
                  get_next_s_chr();
                  get_next_s_chr();
                  get_next_s_chr();
                  ret = MATCHED;
                } 
                while (is_xdigit(s_chr)) {
                  ret = MATCHED;
                  get_next_s_chr();
                }
                break;

      case 'D' : // Integer number 
                intnumber = true;

      case 'F' : // Floating point number
                if (s_chr == '+' || s_chr == '-') {
                  do {
                    get_next_s_chr();
                  } while (is_space(s_chr));
                } 
  
                while (is_digit(s_chr)) {
                  ret = MATCHED;
                  get_next_s_chr();
                }

                if (intnumber) break;

                if (s_chr == '.') {
                  get_next_s_chr();
                }

                while (is_digit(s_chr)) {
                  ret = MATCHED;
                  get_next_s_chr();
                }
  
                if ((ret == MATCHED) && (s_chr == 'E' || s_chr == 'e')) {
                  get_next_s_chr();
                  if (s_chr == '+' || s_chr == '-')  get_next_s_chr();
                  while (is_digit(s_chr)) get_next_s_chr();
                  if (s_chr == '.') get_next_s_chr();
                  while (is_digit(s_chr)) get_next_s_chr();
                }

                break;

      default  : ret = MATCHED_FAIL; pat--; break;
    }
    p_end = pat;
  }
  else {
    p_chr = skp_next(pat,&p_end,iso);
    s_end = s_tmp;
    ret = chr_cmp(s_chr,p_chr,fold); 
  }

  if (ret != MATCHED_FAIL) {
    if (pat_end) *pat_end = p_end;
    if (src_end) *src_end = s_end;
  }
  return ret;
}

// skp("&*!s")  skp("&>&s")

char *skp_(char *pat, char *src, char **end, int *alt)
{
  char *start = src;
  char *s;
  char *p;
  char *s_end;
  char *p_end;
  int   skpto = 0;
  int   matched = 0;
  char *goal = NULL;
  char *goalnot = NULL;

  if (!pat || !src ) { errno = 1; return src; }
  
  if (alt) *alt = -1;

  if (pat[0] == '&' && pat[1] == '>') {
    skpto = 1;
    pat += 2;
  }

  p = pat;
  s = start;
  while (*p >= '\7') {
    if ((matched = match(p,s,&p_end,&s_end))) {
     _dbgtrc("matched( '%s' '%s'",s,p);
      s = s_end; p = p_end;
     _dbgtrc("matched) '%s' '%s'",s,p);
      if (matched == MATCHED_GOAL && !goalnot) goal = s;
      else if (matched == MATCHED_GOALNOT) goalnot = s;
    }
    else {
      while (*p > '\7') p++;
      if (*p > '\0') {
        s = start;
        p++;
       _dbgtrc("resume from: %s (%c)", p,*s);
      }
      else if (skpto) {
        goal = NULL;  goalnot = NULL;
        p = pat;
        s = ++start;
        if (*s == '\0') break;
      }
      else break;
    }
  }
 _dbgtrc("pat: '%s'",p);

  if (!matched && goalnot) {
    goal = goalnot;
    matched = MATCHED;
    p="";
  }

  if (goal) s = goal;

  if (matched && (*p <= '\7')) {
    errno = 0;
    if (alt) *alt = *p;
    if (end) *end = s;
    if (skpto) s = start;
    return s;
  }
  
  errno = 1;
  if (end) *end = src;
  return src;
}

#endif // SKP_MAIN
#endif // SKP_VER