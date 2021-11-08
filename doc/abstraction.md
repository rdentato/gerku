# Abstraction algorithm

  In [1], Kerby discuss *abstraction* by converting expressions
containg concatenative combinators to lambda expressions with 
variables and then proceeds by abstracting variables from the
lambda expressions.

  This article follows a more direct approach and provides
abstraction rules that are aimed to ease the implementatocan be 
directly applied to expressions.


## Abstraction
 
The result of the *abstraction* of the variable `@` from the
expression `F` is an expression that does not contain `@` and
that, when applied to `(@)` will return `F`:

```
           G = {F}[(@)]  ->  (@) G = F
```

The *abstraction* of an expression `F` with respect to the
variable `@` is denoted with `{F}[(@)]`.

In a sense, abstraction is similar to compilation. Given
an expression `F` containing variables (the *source code*)
we can see `{F}[(@)]` as a program that when applied to a
term `(x)` will result in the original expression where the
variable is replaced by `(x)`.

When abstracting multiple variable we'll have:

```
         {F}[(@1) (@2)] = {{F}[(@1)]}[(@2)]
```

Note that we are only interested to show that such algorithm exists
whether is a practical one or not (abstraction usually leads to very
long and complex expressions) is a different issue.

## Combinators

  We'll use the following combinators:
 
```
                    (@) i    = @1
                (@) (@) k    = @2
                (@) (@) cons = ((@1) @2)
                (@) (@) dip  = @2 (@1)
                (@) (@) sip  = (@1) @2 (@1)
```

  Being able to represent any expression with the 
combinators proves that they form a (non minimal) base.

  A note on the syntax. Rather than having *named* variables 
as in [1], we'll use `@` to signify a variable. If followed by
a number, it's the position in which they appear.

  A definition like:

```
       (@) (@) YY = (@2) (@1 XX)
```

  can be interpreted both in terms of stack and as
a rewrite rule.

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
expressions. We'll use the following definitions:

 - An *expression* is a list of terms and variables
 - A *combinator* is a term
 - A *variable* is a term
 - The *nil* quote `()` is a term
 - An expression enclosed in parenthesis is a term (quote)
 - An *atom* is a combinator or a variable

 - `@`       is a generic variable
 - `#..#`    is a sequence of terms that *do not* contain `@`
             (i.e. `@` does not occur in `$...$`)
 - `$..$`    is a sequence of terms that *may* contain `@`
             (i.e. `@` may occur in `$...$`)
 - `%..%`    is a sequence of terms that *does* contain `@`
             (different from `$...$`)


  We'll consider the expression as a list of terms and will apply the following
abstraction rules.

```
     0      {$..$ #..#}[(@)] = {$..$}[(@)] #..#

     1      {#..# $..$}[(@)] = (#..#) dip {$..$}[(@)]
     1a        {(#..#)}[(@)] = ((#..#)) k 
     1b          {#..#}[(@)] = (#..#) k
     1c            {()}[(@)] = (()) k
     1d              {}[(@)] = () k

     2    {(%..%) $..$}[(@)] = ({(%..%)}[(@)]) sip {$..$}[(@)]
     2a      {(@) $..$}[(@)] = () sip {$..$}[(@)]
     2b        {($..$)}[(@)] = ({$..$}[(@)]) cons
     2c           {(@)}[(@)] = 

     3         {@ $..$}[(@)] = (i) sip {$..$}[(@)]
     3a             {@}[(@)] = i

```

  Note that the expressions on the right side can't be reduced further
without being applied to a quoted argument.

### Rule 1
  This rule allows us to stop earlier in the abstraction process: trailing
terms not containing `@` can be left untouched.

```
     0      {$..$ #..#}[(@)] = {$..$}[(@)] #..#
```

  Let's check that rule `0` holds:
```
     (@) {$..$}[(@)] #..#
     ^-------------^  by definition of abstraction
     $..$ #..#
```

### Rule 1
  This rule is to be applied when the expression consists of a list
of terms which do not contain `@` followed by a list of terms which
may contain `@`.

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

  It's easy to verify that rules `1a` to `1d` hold by applying
them to `(@)` as we did for rule `1`.

  It can be also verified that rule `1` is still applicable in 
those cases and that `1a` to `1d` are just special rules to 
simplify the algorithm. Let's do it fo `1b` in which the
list `$..$` is empty:

```
       {#..# $..$}[(@)] = (#..#) dip {$..$}[(@)]
       {#..#}[(@)] = (#..#) dip {}[(@)]
```

  Now let's apply it to `@`:

```
       (@) (#..#) dip {}[(@)]
       ^------------^             by def. of dip
       #..# (@) {}[(@)]
            ^---------^           by def. of abstraction
       #..#
```

  The other cases can be verified in a similar way.

### Rule 2
  This rule is to be applied when the expression consist of
a quote (wich contains `@`) follwed by a list of terms which
may contain `@`.

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
```

  Rules `2a` to `2d` are special cases of rule two `2`.
  Actually rule `2` would also work if the quote at the beginning 
would *not* contain `@` (i.e. we could do without rule `1`.)

```
      {(%..%) $..$}[(@)] =
             ({(%..%)}[(@)]) sip {$..$}[(@)]
               ^----^        by hypotesis
             ({(#..#)}[(@)]) sip {$..$}[(@)]
              ^-----------^  
             (((#..#)) k) sip {$..$}[(@)]
```

```
             (@) (((#..#)) k) sip {$..$}[(@)]
             ^------------------^
             (@) ((#..#)) k (@) {$..$}[(@)]
             ^------------^
             (#..#) (@) {$..$}[(@)]
                    ^-------------^
             (#..#) $..$       
```


### Rule 3

   This is the particular case when the first term is `@`.

```
     3         {@ $..$}[(@)] = (i) sip {$..$}[(@)]
```

   To show that rules 3 holds, let's apply it to `(@)`:

```
               (@) (i) sip {$..$}[(@)]
               ^---------^
               (@) i (@) {$..$}[(@)]
               ^---^
               @ (@) {$..$}[(@)]
                 ^-------------^
               @ $..$
```


## Bibliography

[1]  *The Theory of Concatenative Combinators*,
     Brent Kerby (bkerby at byu dot net).
     Completed June 19, 2002. Updated February 5, 2007.
     ([link](http://tunes.org/~iepos/joy.html))


[2]  *Lambda-Calculus and Combinators, an introduction*,
     J. Roger Hindley, Jonathan P. Seldin
     ([link](http://www.cambridge.org/9780521898850))
