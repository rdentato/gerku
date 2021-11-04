# Abstraction algorithm

## Definitions

 - An **expression** is a sequence of terms and variables
 - A **combinator** is a term
 - A **variable** is a term
 - The *nil* quote `()` is a term
 - An **expression** enclosed in parenthesis is a term (quote)
 - An **atom** is a combinator or a variable

 - `@`       is a generic variable
 - `#`       is an atom which is not `@`
 - `$..$`    is a sequence of atoms some of which can be `@`
           (i.e. `@` may occur in `$...$`)
 - `%..%`    is a sequence of atoms some of which can be `@`
           (i.e. may occur in `%...%`)
 
The **abstraction** of an expression `F` with respect to the
variable `@` is denoted with `{F}[@]`.

The result of the abstaction of `@` from `F` is an expression
with no occurences of the variable `@` that, when applied to
`@` will return `F`:

           G = {F}[@]  ->  @ G = F

In a sense, abstraction is similar to compilation.
Given an expression that contains variables, the abstraction process
will return an expression with no variables such that applying
that expression to a list of arguments will return the same result
as binding those arguments to the variables in the original expression.

When abstracting multiple variable we'll have:

            {F}[@1 @2] = {{F}[@1]}[@2]

Note that we are only interested to show that such algorithm exists
whether is a practical one or not (abstraction usually leads to very
long and complex expressions).

## Combinators
  We'll use the following combinators:
 
```
               (@) I = @1       // Identity
               (@) D = @1 @1    // Dup
           (@) (@) K = @2       // Kill (Delete)
           (@) (@) E = @2 @1    // Exchange (Swap)
           (@) (@) J = (@1 @2)  // Join (concat)
               (@) Q = ((@1))   // Quote quotes
                @  q = (@1)     // Quote
       (@) (@) (@) S = (@1 @2) @1 @3 
```

  Being able to represent any expression with the 
combinators proves that they form a (non minimal) base.

  Note the difference between the combinators `Q` and `q`.
The first one add an extra parentesis only to quotes,
the second one to any term to its left.

## The algorithm
  To find an algorithm we'll reason on the structure of expressions.

       1              {}[@] = q () K
       2            {()}[@] = q (()) K
       3             {@}[@] = q I
       4           {(@)}[@] = q
       5             {#}[@] = q (#) K
       6           {(#)}[@] = q ((#)) K
       7        {# $..$}[@] = q ({$...$}[@]) J (#) E J I
       8        {@ $..$}[@] = q Q D ({$..$}[@]) J J I
       9   {(%..%) $..$}[@] = q ({%..%}[@]) ({%..%}[@]) S

  Note that the expressions on the right side can't be reduced further
  without being applied to an argument.

  Rules 1-6 are easily verified. Next sections show that by apply the
rules to the variable `@` we'll get the expected result

### Rule 1

      @ q () K
      (@) () K
               <- nothing

### rule 2

      @ q (()) K
      (@) (()) K
      ()

### rule 3
      @ q I
      (@) I
      @

### rule 4
      @ q
      (@)

### rule 5
      @ q (#) K
      (@) (#) K
      #

### rule 6
      @ q ((#)) K
      (@) ((#)) K
      (#)

### rule 7
      @ q ({$...$}[@]) J (#) E J I
      (@) ({$...$}[@]) J (#) E J I
      (@ {$...$}[@]) (#) E J I
      (#) (@ {$...$}[@]) J I
      (# @ {$...$}[@]) I
      # @ {$...$}[@]
      # $..$

### rule 8

      @ q Q D ({$..$}[@]) J J I
      (@) Q D ({$..$}[@]) J J I
      ((@)) D ({$..$}[@]) J J I
      (@) (@) ({$..$}[@]) J J I
      (@) (@ {$..$}[@]) J I
      (@ @ {$..$}[@]) I
      @ @ {$..$}[@]
      @ $..$

### rule 9

      @ q ({%..%}[@]) ({%..%}[@]) S
      (@) ({%..%}[@]) ({%..%}[@]) S
      (@ {%..%}[@]) @ {%..%}[@]
      (@ {%..%}[@]) $..$
      (%..%) $..$  

  Note that last step will not really be computed
since quotes are opaque. However we know (by definition)
that the result within the quote is the same.
   
