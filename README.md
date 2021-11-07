# gerku

```
                     _,            
                    | |            
   __ _,  ___ ,_ __ | | __ _,  _,     /\___     
  / _` | / _ \| '__)| |/ /| | | |    (    @\___ 
 ( (_) |(  __/| |   |   ( | |_| |   /          )
  \_,  | \___)|_|   |_|\_\ \__,_|  /    (_____/ 
  __/  |                          (_____/    U  
 (____/ 

```
  An interepreter for Concatenative Combinators as described in:

  "The Theory of Concatenative Combinators by Brent Kerby"
  ([link](http://tunes.org/~iepos/joy.html#applic))
 
  `gerku` started as sidekick project of `mlatu` to check if using
stack based semantics would be equivalent to using rewriting rules.
Given the type of rewriting rules `mlatu` is aiming to handle, the
answer is *NO*!

  mlatu is a project by Caden Haustein (carmysilna)
  ([link](https://github.com/mlatu-lang/mlatu))

  `gerku` is now only aimed to be a tool for those wanting to explore
the concatenative Combinatory Logic described in the Kerby's article.

## Compile & Run

  Clone (or fork) the repository on [Github](https://github.com/rdentato/gerku).

  In the `src` directory just type `make` (tested on linux and on Windos MSYS2)

  ```
     prj/gerku/src> make
     prj/gerku/src> ./gerku
  ```

  All dependencies are in the `src/libs` directory, all libraries used 
are by me except [`linenoise`](https://github.com/antirez/linenoise), a small,
fast, self-contained, MIT-licensed, replacement of  `readline`.

  If the terminal you're using has issues with `linenoise`, you
can change the settings in the makefile comment the line:
  ```
  LINENOISE=-DUSE_LINENOISE
  ```
and recompile. You will lose history and line editing features.

## Command line
   `gerku` has few options that can be specified on the command line:

```
prj/gerku/src> ./gerku -h
GERKU 0.0.5-beta (C) 2021 https://github.com/rdentato/gerku
Usage: gerku [OPTIONS] ...
   -d           Delete default combinators
   -r           Run mode (no REPL)
   -v           Version
```

All arguments after the options are considered to be source files
to be loaded and executed before entering the REPL.
The directory `grk` has some example.

## The REPL

The list of REPL commands is accessible within the REPL iteself:

```
prj/gerku/src> ./gerku
GERKU 0.0.5-beta (C) 2021 https://github.com/rdentato/gerku
Type ! for available commands.
|-> 
gerku> !
Available commands:
  !help              this help
  !list              list of defined words
  !load file         load definitions from file
  !save file         save definitions to file
  !print             print current stack
  !trace             toggle reduction tracing
  !quit              exit the repl
  !def ...           define a new word
  !del word | !all   delete a word [all words!]
  !wipe [auto]       wipe the stack [and toggles autowipe]
|-> 

```

## Defining a combinator

  A new combinator is defined through the "!def" command:

```
prj/gerku/src> ./gerku -h
./gerku -d
GERKU 0.0.5-beta (C) 2021 https://github.com/rdentato/gerku
Type ! for available commands.
|-> 
gerku> !def (@) (@) COMB = ((@1) @2)
|-> 
gerku> !list
        (@) (@) COMB = ((@1) @2)
|-> 
gerku> (x) (y) COMB
|-> ((x) y) 
```

## Kerby's combinators

By default `gerku` only defines few ad-hoc combinators
that were useful in the first stage of the project.

To load the combinators used in Kerby's article just
start `gerku` with:

```
  prj/gerku/src> ./gerku -d grk/kerby.grk
```
