# Concatenative Combinators abstraction algorithm

  In [1], Kerby discuss *abstraction* by converting expressions
containg concatenative combinators to lambda expressions with 
variables and then proceeds by abstracting variables from the
lambda expressions.

  This article follows a more direct approach and provides
abstraction rules that are aimed to make the algorithm easier
to apply and implement.


## Abstraction
 
  The *abstraction* of an expression `F` with respect to the
variable `@` is denoted with `{F}[(@)]`.
  The result of the *abstraction* of the variable `@` from the
expression `F` is an expression `G` that does not contain `@`
and that, when applied to `(@)`. will return `F`:

```
           G = {F}[(@)]  ->  (@) G = F
```

  In a sense, abstraction is similar to compilation. Given
an expression `F` containing variables (the *source code*)
we can see `{F}[(@)]` as a program that when applied to a
term `(x)` will result in the original expression where the
variable is replaced by `(x)`.

  When abstracting multiple variable we'll have:

```
         {F}[(@1) (@2)] = {{F}[(@1)]}[(@2)]
```

  In other words, we first abstract wrt `@1`, then `@2` and so on.

  Note that we are only interested in showing that such algorithm exists
whether is a practical one or not (abstraction usually leads to very
long and complex expressions) is a different issue.

## Combinators

  We'll use the following combinators:
 
```
                    (@) i    = @1
                    (@) zap  = 
                    (@) run  = @1 (@1)
                    (@) dup  = (@1) (@1)
                (@) (@) cons = ((@1) @2)
                (@) (@) dip  = @2 (@1)
                (@) (@) sip  = (@1) @2 (@1)
```

  Rather than having *named* variables as in [1], we'll use `@`
to denote variables. If followed by a number, it's the position
in which they appear in the list of arguments.

  A definition like:

```
       (@) (@) YY = (@2) (@1 XX)
```

  can be interpreted both in terms of stack (with top being the
leftmost term of the expression) and as a rewrite rule.

  - *As a rewrite rule* :  if the current expression matches 
  two quoted terms followed by `YY`, it is replaced by the left
  hand expression in which `@1` and `@2` are replaced by the 
  two matching terms.

  - *As stack operations* : when `YY` is on top of the stack and
  the two element below are quotes, then pop the two elements and
  push the left hand terms after replacing `@x` with the terms
  that were in the stack. 

## Abstraction rules
  To find an algorithm we'll reason on the structure of
expressions that have the following grammar:

```
   expression := term+
   term := comb | var | quote
   quote := '(' expression? ')'
   comb := [A-Za-z_][A-Za-z_0-9]*
   var := '@' num
   num := [0-9]+
```

 That can be summarized as:

 - An *expression* is a list of terms
 - A *combinator* is a term
 - A *variable* is a term
 - The *nil* quote `()` is a term
 - A *quote* (an expression enclosed in parenthesis) is a term


 - `@`       is a generic variable
 - `#..#`    is a sequence of terms that *does not* contain `@`
             (i.e. `@` does not occur in `$...$`)
 - `$..$`    is a sequence of terms that *does* contain `@`
             (i.e. `@` occurs in `$...$`)
 - `%..%`    is a sequence of terms that *does* contain `@`
             (different from `$...$`)

  Given an expressions, looking at the list of terms from left to
right there can only be the following cases:

  0. The expression can be split in two parts with first terms
     containing the variable `@` followed by other terms not
     containing `@`

  1. The expression can be split in two parts with first terms
     not containing the variable `@` followed by other terms
     containing `@`
     
  2. The first term is a quote containing `@` (otherwise it 
     we would accounted for in case 1) followed by other terms
     possibly containing `@`

  3. The fist term is the variable `@`

  This leads to the following abstraction rules:

```
     0      {%..% #..#}[(@)] = {%..%}[(@)] #..#

     1      {#..# %..%}[(@)] = (#..#) dip {%..%}[(@)]
     1a          {#..#}[(@)] = zap #..#
     1b              {}[(@)] = zap

     2    {(%..%) $..$}[(@)] = ({(%..%)}[(@)]) sip {$..$}[(@)]
     2a      {(@) $..$}[(@)] = dup {$..$}[(@)]
     2b        {(%..%)}[(@)] = ({%..%}[(@)]) cons
     2c           {(@)}[(@)] = 

     3         {@ $..$}[(@)] = run {$..$}[(@)]
     3a             {@}[(@)] = i
     
```

  Note that the expressions on the right side can't be reduced further
without being applied to a quote.

  It's easy to prove, by induction on the length of the expressions,
that the algorithm converges: at each step the expressions to be
abstracted become smaller and smaller.

  In the following subsection we'll show that rules do hold by applying
them to `(@)` and checking that the result is, indeed, the original expression.

### Rule 0
  This rule allows us to stop earlier in the abstraction process: trailing
terms not containing `@` can be left untouched.

  This is implied by the fact that the combinators are concatenative.

```
     0      {$..$ #..#}[(@)] = {$..$}[(@)] #..#
```

  Let's check that rule `0` holds:
```
     (@) {$..$}[(@)] #..#
     ^-------------^        by definition of abstraction
     $..$ #..#
     ^-------^              by definition of abstraction
     {$..$ #..#}[(@)]
```

### Rule 1
  This rule is to be applied when the expression consists of a list
of terms which do not contain `@` followed by a list of terms which
contain `@`.

```
     1      {#..# $..$}[(@)] = (#..#) dip {$..$}[(@)]
```

  To prove that this rule holds, let's apply it to `@` and check 
that the result is `#..# $..$`.

```
       (@) (#..#) dip {$..$}[(@)]
       ^-------------^                by def. of dip
       #..# (@) {$..$}[(@)]
            ^-------------^           by def of abstraction
       #..# $..$
       ^-------^                      by def of abstraction
       (@) {#..# $..$}[(@)]
```

  Rule 1 works even if one or both the lists of terms
`#..#` and `$..$` are empty:

    - `#..#` is empty:
```
       (@) () dip {$..$}[(@)]
       ^--------^                by def. of dip
       (@) {$..$}[(@)]
       ^-------------^           by def of abstraction
```

    - `$..$` is empty:
```
       (@) (#..#) dip {}[(@)]
       ^-------------^            by def. of dip
       #..# (@) {}[(@)]
            ^---------^           by def of abstraction
       #..#
       ^--^                       by def of abstraction
       (@) {#..#}[(@)]
```

    - both `#..#` and `$..$` are empty: 
```
       (@) () dip {}[(@)]
       ^-------------^            by def. of dip
       (@) {}[(@)]
       ^---------^                by def of abstraction
                                  <-- empty expression
```

  Rules `1a` and `1b` just simplify the expression by using 
the `zap` combinator.


### Rule 2
  This rule is to be applied when the expression consist of
a quote that contains `@` followed by a list of terms which
contain `@` (if they didn't we would have used rule 0).

```
     2    {(%..%) $..$}[(@)] = ({(%..%)}[(@)]) sip {$..$}[(@)]
```

  Let's apply it to `@`:

```
       (@) ({(%..%)}[(@)]) sip {$..$}[(@)]
       ^---------------------^                by def. of sip
       (@) {(%..%)}[(@)] (@) {$..$}[(@)]
       ^---------------^                      by def. of abstraction
       (%..%) (@) {$..$}[(@)]
              ^-------------^                 by def. of abstraction
       (%..%) $..$
       ^---------^                            by def. of abstraction
       (@) {(%..%) $..$}[(@)]
```

  Rules `2a` to `2d` are special cases of rule `2`.
  

### Rule 3

   This is the rule to apply when the first term is `@`.

```
     3         {@ $..$}[(@)] = run {$..$}[(@)]
```

   To show that rules 3 holds, let's apply it to `(@)`:

```
               (@) run {$..$}[(@)]
               ^-----^                by def. of run
               @ (@) {$..$}[(@)]
                 ^-------------^      by def. of abstraction
               @ $..$
               ^----^                 by def. of abstraction
               (@) {@ $..$}[(@)]
```


## Bibliography

[1]  *The Theory of Concatenative Combinators*,
     Brent Kerby (bkerby at byu dot net).
     Completed June 19, 2002. Updated February 5, 2007.
     ([link](http://tunes.org/~iepos/joy.html))


[2]  *Lambda-Calculus and Combinators, an introduction*,
     J. Roger Hindley, Jonathan P. Seldin
     ([link](http://www.cambridge.org/9780521898850))
