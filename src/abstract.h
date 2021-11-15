#ifndef ABSTRACT_H
#define ABSTRACT_H

#define abstract(...) vrg(abstract_,__VA_ARGS__)
#define abstract_1(expr)             abstract_expr(expr, 0, NULL, 0)
#define abstract_2(expr,var)         abstract_expr(expr, var, NULL, 0)
#define abstract_3(expr,var,wrd)     abstract_expr(expr, var, NULL, 0)
#define abstract_4(expr,var,wrd,len) abstract_expr(expr, var, wrd, len)

char *abstract_expr(char *expr, int var, char *word, int wrd_len);

#endif