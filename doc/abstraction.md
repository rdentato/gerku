# Concatenative Combinators abstraction algorithm

## Abstract
  In [1], Kerby discuss *abstraction* by converting expressions containg concatenative combinators to lambda expressions with variables and then proceeds by abstracting variables from the lambda expressions.

  This article follows a more direct approach and provides abstraction rules that are aimed to make the algorithm easier to apply and implement.

  Even if we are mostly interested in showing that such algorithm exists we have made an effort to choose a base of combinators that will keep the resulting expression as short as possible (abstraction usually leads to very long and complex expressions). Further improvements can be done while implementing the algorithm, but we won't discuss them here.

## Expressions
  The language of concatenative combinators is defined by the following EBNF:

```
   expression := term+
   term := combinator | var | quote
   quote := '(' expression? ')'
   combinator := [A-Za-z_][A-Za-z_0-9]*
   var := [𝑎𝑏𝑐-𝑥𝑦𝑧]
```

 That can be summarized as:

 - An *expression* is a list of terms
 - A *combinator* is a term
 - A *variable* is a term
 - A *quote* (an expression enclosed in parenthesis) is a term
 - The *nil* quote `()` is a term

  Note that variables are notated with a different set of symbols.

  Some examples:
```
   (𝑦) dup
   (cons (𝑧)) sip (𝑦)
   (alpha) (beta) dip
```

  The *evaluation* of an expression proceeds from left to right by finding the first combinator that can be successfully reduced (i.e. the one that has enough quotes at its left) and replacing it and its arguments with the result of its application to the arguments.
  
  We  assume that, as in classical Combinatory Logic, the Church-Rosser theorem holds. While not a real proof, at the end of this paper we'll provide a reasoning that reassure us on the validity of the Church-Rosser theorem.

## Combinators
  Let's use an intuitive definition of what a combinator is:

  > A *combinator* is an operator that shuffles, duplicates or removes
  > *quotes* there are at his left in an expression.

  The behaviour of a combinator is defined as an equivalence like this:

```
    (𝑦) (𝑥) swap  = (𝑥) (𝑦)
```

  You can interpret the definition of `swap` above by saying that in any given expression you can replace any occurence of:
```
    (𝑦) (𝑥) swap
```
with:
```
    (𝑥) (𝑦)
```

  For the sake of reasoning (and as a possible strategy of implementation) we can think of a combinator as a program that operates on a stack:

  > A combinator is a *program* that pulls a certain number of quoted expressions from a stack and push back in the stack a number of expressions containing any number of them (even zero), possibily in a different order

  To simplify the abstraction algorithm we'll use the following combinators:
 
```
                    (𝑥) i    = 𝑥
                    (𝑥) zap  = 
                    (𝑥) run  = 𝑥 (𝑥)
                    (𝑥) dup  = (𝑥) (𝑥)
                (𝑦) (𝑥) cons = ((𝑦) 𝑥)
                (𝑦) (𝑥) cosp = ((𝑦) 𝑥) (𝑦)
                (𝑦) (𝑥) dip  = 𝑥 (𝑦)
                (𝑦) (𝑥) sip  = (𝑦) 𝑥 (𝑦)
```

which are a superset of the bases defined in [1] and, hence, ensure that every possible expression can be represented using those combinators.

  We'll extend the concept of combinator *defintions* by allowing a combinator to add other elements that were not in the original arguments.

  For example:
```
              (𝑦) (𝑥) add = (𝑦) (succ) 𝑥
             (𝑦) (𝑥) mult = (zero) ((𝑦) add) 𝑥
```

## Abstraction
 
  The *abstraction* of an expression `𝓕` with respect to the variable `𝑥` is denoted with `{𝓕}[(𝑥)]`.

  The result of the *abstraction* of the variable `𝑥` from the expression `𝓕` is an expression `𝓖` that does not contain `𝑥` and that, when applied to `(𝑥)`. will return `𝓕`:

```
           𝓖 = {𝓕}[(𝑥)]  ->  (𝑥) 𝓖 = 𝓕
```

  In a sense, abstraction is similar to compilation. Given an expression `𝓕` containing a variable `𝑥` (the *source code*) we can see `{𝓕}[(𝑥)]` as a program that when applied to a quoted expression `(𝓡)` will result in the original expression where any occurence of the variable `𝑥` is replaced by `𝓡`.

  When abstracting multiple variable we'll have:

```
         {𝓕}[(𝑦) (𝑥)] = {{𝓕}[(𝑥)]}[(𝑦)]
```

  In other words, we first abstract wrt `𝑥`, then `𝑦` and so on.

  Note that abstraction is defined with respect to a quoted variable as concatenative combinators are defined to only operate on quotes.


## Abstraction rules

  We'll use the following definitions:

 - `𝑥` is a generic variable
 - `𝓖` is a non empty expression that *does not* contain `𝑥`
       (i.e. `𝑥` *does not occur* in `𝓖`)
 - `𝓜` is an expressions that *may* contain `𝑥`
       (`𝑥` *may occur* in `𝓜`)
 - `𝓝` is an expressions that *do* contain `𝑥`
       (`𝑥` *occurs* in `𝓝`)

  Given an expression, looking at the list of terms from left to right there can only be the following cases:

  1. The expression is empty;

  2. The expression can be split in two parts with first terms containing the variable `𝑥` followed by other terms not containing `𝑥`;

  3. The expression can be split in two parts with first terms not containing the variable `𝑥` followed by other terms containing `𝑥`;
     
  4. The first term is a quote containing `𝑥` (otherwise it would have been accounted for in case 3) followed by an expression containing `𝑥` (otherwise it would have been accounted for in case 0);

  5. The fist term is the variable `𝑥` (unquoted).

  This leads to the following abstraction rules:

```
     1           {}[(𝑥)] = zap
     2       {𝓝 𝓖}[(𝑥)] = {𝓝}[(𝑥)] 𝓖
     3       {𝓖 𝓝}[(𝑥)] = (𝓖) dip {𝓝}[(𝑥)]
     4    {(𝓝) 𝓜}[(𝑥)] = ({𝓝}[(𝑥)]) cosp {𝓜}[(𝑥)]
     5       {𝑥 𝓜}[(𝑥)] = run {𝓜}[(𝑥)]
     
```

  Note that the expressions on the right side can't be reduced further without being applied to a quote.

  It's easy to prove, by induction on the length of the expressions, that the algorithm converges: at each step the expressions to be abstracted become smaller and smaller.

  In the following subsection we'll show that the rules do hold by applying them to `(𝑥)` and checking that the result is, indeed, the original expression.

### Rule 1
  This is the base case for when the expression is empty.

```
     1           {}[(𝑥)] = zap     
```
  
  Let's check that applying the result to (𝑥) we got the empty expression:

```
       (𝑥) zap
       ╰─────╯            by def. of zap
                      ◄── empty expression
       ╰╯                 by def. of abstraction
       (𝑥) {}[(𝑥)]
```


### Rule 2
  This rule allows us to stop earlier in the abstraction process: trailing terms not containing `𝑥` can be left untouched.

  This is implied by the fact that the combinators are concatenative.

```
     2      {𝓝 𝓖}[(𝑥)] = {𝓝}[(𝑥)] 𝓖
```

  Let's check that rule `2` holds:

```
     (𝑥) {𝓝}[(𝑥)] 𝓖
     ╰───────────╯        by definition of abstraction
     𝓝 𝓖
     ╰──╯                 by definition of abstraction
     (𝑥) {𝓝 𝓖}[(𝑥)]
```

### Rule 3

  This rule is to be applied when the expression consists of a list of terms which do not contain `𝑥` followed by a list of terms which contain `𝑥`.

```
     3      {𝓖 𝓝}[(𝑥)] = (𝓖) dip {𝓝}[(𝑥)]
```

  To prove that this rule holds, let's apply it to `(𝑥)` and check that the result is `𝓖 𝓜`.

```
       (𝑥) (𝓖) dip {𝓝}[(𝑥)]
       ╰─────────╯                by def. of dip
       𝓖 (𝑥) {𝓝}[(𝑥)]
         ╰───────────╯            by def of abstraction
       𝓖 𝓝
       ╰──╯                       by def of abstraction
       (𝑥) {𝓖 𝓝}[(𝑥)]
```

### Rule 4
  This rule is to be applied when the expression consist of
a quote that contains `𝑥` followed by a list of terms which
contain `𝑥` (if they didn't we would have used rule 0).

```
     4    {(𝓝) 𝓜}[(𝑥)] = ({𝓝}[(𝑥)]) cosp {𝓜}[(𝑥)]
```
  Let's apply it to `(𝑥)`:

```
       (𝑥) ({𝓝}[(𝑥)]) cosp {𝓜}[(𝑥)]
       ╰──────────────────╯                by def. of cosp
       ((𝑥) {𝓝}[(𝑥)]) (𝑥) {𝓜}[(𝑥)]
        ╰───────────╯                      by def. of abstraction
       (𝓝) (𝑥) {𝓜}[(𝑥)]
           ╰────────────╯                  by def. of abstraction
       (𝓝) 𝓜
       ╰─────╯                             by def. of abstraction
       (𝑥) {(𝓝) 𝓜}[(𝑥)]
```

### Rule 5

   This is the rule to apply when the first term is `𝑥`.

```
     5         {𝑥 𝓜}[(𝑥)] = run {𝓜}[(𝑥)]
```

   To show that rules `5` holds, let's apply it to `(𝑥)`:

```
               (𝑥) run {𝓜}[(𝑥)]
               ╰─────╯              by def. of run
               𝑥 (𝑥) {𝓜}[(𝑥)]
                 ╰───────────╯      by def. of abstraction
               𝑥 𝓜
               ╰──╯                 by def. of abstraction
               (𝑥) {𝑥 𝓜}[(𝑥)]
```

## Optimization

  The only optimization we mention here is the possibility of
simplifying some special cases:

``` 
     3a         {𝓖}[(𝑥)] = zap 𝓖
     4a    {(𝑥) 𝓜}[(𝑥)] = dup {𝓜}[(𝑥)]
     4b      {(𝓝)}[(𝑥)] = ({𝓝}[(𝑥)]) cons
     4c       {(𝑥)}[(𝑥)] = 
     5a         {𝑥}[(𝑥)] = i
     
```

  They can be easily checked as we did in the previous section.

  Note that those special cases are included in the general case when one of the expressions is empty or has a special form.

  Let's give just one example that shows that rule `4b` is *implied* in rule `4` when the expression 𝓜 is empty.

```
     4    {(𝓝) 𝓜}[(𝑥)] = ({𝓝}[(𝑥)]) cosp {𝓜}[(𝑥)]
     4b      {(𝓝)}[(𝑥)] = ({𝓝}[(𝑥)]) cons
```

  It's easy to see that, under the assumption of 𝓜 being empty, the two are equivalent:

```

  (𝑥) {(𝓝) 𝓜}[(𝑥)]
      ╰────────────╯                 by rule 4
  (𝑥) ({𝓝}[(𝑥)]) cosp {𝓜}[(𝑥)]
                      ╰────────╯     by hypotesis that 𝓜 is empty
  (𝑥) ({𝓝}[(𝑥)]) cosp {}[(𝑥)]
  ╰──────────────────╯               by def. of cosp
  ((𝑥) ({𝓝}[(𝑥)])) (𝑥) {}[(𝑥)]
                       ╰──────╯      by rule 1
  ((𝑥) ({𝓝}[(𝑥)])) (𝑥) zap
                   ╰──────╯          by def. of zap
  ((𝑥) ({𝓝}[(𝑥)]))
  ╰───────────────╯                  by def. of cons
  (𝑥) ({𝓝}[(𝑥)]) cons
  ╰──────────────────╯               by rule 4b
  (𝑥) {(𝓝)}[(𝑥)] 

```
## Quotes
  As said at the beginning. combinators only operate on quotes.

  This is need since (as in CL and 𝜆-calculus) there is no distinction between functions (programs) and data. Quotes are what make this distinction.

  Note that we have assumed that quotes are *transparent*. i.e. that reductions may happen within a quote. This has the advantage that all expressions are reduced to their minimal form but also has the disadvantage of having to compute any single quotes even if they might be discarded at a later time. *Opaque* quotes allow for lazy evaluation and the content of a quote is evaluated only when (and if) it is really need, not before.

  From the abstraction algorithm there is no difference as the two type of quotes are equivalent.

## The Church-Rosser theorem
  The Church-Rosser theorem, plays a key role in the evaluation of an expression and the fact that it holds for concatenative combinators is an assumption for the abstraction algorithm to work. It is also essential for the discussion on transparte/opaque quotes in the preceding section.

  It can be formulated in many different ways, this is one of them:

  > Let 𝓐 ⊳ 𝓑 be the reduction of the expression 𝓐 to the expression 𝓑; then
  >      𝓤 ⊳ 𝓧 ∧ 𝓤 ⊳ 𝓨 ⇒ ∃𝓩:  𝓧 ⊳ 𝓩 ∧ 𝓨 ⊳ 𝓩

  In plain words, if an expression 𝓤 can be reduced to two different expressions 𝓧 and 𝓨, then there is an expression 𝓩 to whom both 𝓧 and 𝓨 can be reduced.

  Which means that the strategy of reductions is irrelevant as any of them will lead to the same expression 𝓩.

  While the general proof of this theorem is quite complex, in our specific case (concatenative combinators that only act on quotes) it's pretty straightforwad to convince ourselves that the theorem holds.

  Let's consider the following expression:
  ```
          (𝓊) A (𝓋) B
  ```
  where `A` and `B` are combinators and `𝓊` and `𝓋` are generic expressions.
  
  The only interesting case is when both of them can be reduced (i.e. they both are a *redex*). Let's consider one step of reduction.

  If we reduce `(𝓊) A`, there will be no consequence on `(𝓋) B`, since it is already a redex.

  If we reduce `(𝓋) B` the result can be:
   - a redex itself, which brings us in the same situation we were before the reduction,
   - a non reducible expression like `(𝓈) (𝓉) C`.
   
  The resulting expression `(𝓊) A (𝓈) (𝓉) C` now contains only one redex (`(𝓊) A`) becase the combinator `C` only operates on quotes and `A` is unquoted.
 
  This reasoning can be repeated for a more general case but it's easy to see that the redex in an expressions to do not interfere with each other and, hence, the order in which they are reduced is irrelevant for the end result.


## Conclusion
  We have defined an abstraction algorithm for Concatenative combinators that is simple enough to be implemented and even being applied by hand 

  This frees us from the need of handling variables when using Concatenative Combinators.

  We have also argumented around the validity of the Church-Rosser theorem that is an assumption for the abstraction algorithm to work.

  Here are the list of all the abstraction rules (including the special cases):
  
```
     1           {}[(𝑥)] = zap
     2       {𝓝 𝓖}[(𝑥)] = {𝓝}[(𝑥)] 𝓖
     3       {𝓖 𝓝}[(𝑥)] = (𝓖) dip {𝓝}[(𝑥)]
     3a         {𝓖}[(𝑥)] = zap 𝓖
     4    {(𝓝) 𝓜}[(𝑥)] = ({𝓝}[(𝑥)]) cosp {𝓜}[(𝑥)]
     4a    {(𝑥) 𝓜}[(𝑥)] = dup {𝓜}[(𝑥)]
     4b      {(𝓝)}[(𝑥)] = ({𝓝}[(𝑥)]) cons
     4c       {(𝑥)}[(𝑥)] = 
     5       {𝑥 𝓜}[(𝑥)] = run {𝓜}[(𝑥)]
     5a         {𝑥}[(𝑥)] = i
 

```

## Bibliography

[1]  *The Theory of Concatenative Combinators*,
     Brent Kerby (bkerby at byu dot net).
     Completed June 19, 2002. Updated February 5, 2007.
     ([link](http://tunes.org/~iepos/joy.html))


[2]  *Lambda-Calculus and Combinators, an introduction*,
     J. Roger Hindley, Jonathan P. Seldin
     ([link](http://www.cambridge.org/9780521898850))
