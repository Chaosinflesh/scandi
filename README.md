# The *scandi* Programming Language

*scandi* is a small programming language project I have undertaken in response to a friend's challenge.

The aim of the language is to have no 'keywords' -> though I will note the use of symbols instead could be construed as such.
*scandi* uses every symbol available on a standard keyboard, and a couple of digraphs where appropriate.

*scandi* makes use of spacing to indicate scope changes.

*scandi* uses a table-backed memory management system.

*scandi* provides a small standard library to work with (WIP).

This project is using [LLVM](https://llvm.org) to build a compiler.

## Example Program (Fibonacci Sequence):

```
` fibonacci.scandi

{stream.writeline writeline}
{system.stdout out}

` Calculates the n-th fibonacci value, given two starting values.
$a $b $n @fibonacci

   n 1 <
      "Error: this example is unable to calculate a negative fibonacci index." out writeline
      fibonacci 0 =

   n 1 ?
      fibonacci a =

   $pos 2 =
   $sum a b + =

   \loop
      pos n ?
         fibonacci sum =
      sum a b + =
      a b =
      b sum =
      pos 1 +
      loop


` Calculate the 12th fibonacci number
$result 0 1 12 fibonacci =
"The 12th fibonnacci number is " result + out writeline

```
