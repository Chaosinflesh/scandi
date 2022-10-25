# scandi : a stack-based programming language

***Note: this is a work-in-progress.***

## Overview

* Stack-based : operators follow operands.
* Keyword-less: declarations are operators
* Indentation as scope.
* Table-backed object store.

## Basic Elements

* IDENTIFIERS - alphanumeric words that can be labels, functions, variables or aliases.
* NUMBERS - 64-bit values that are either long or double. Uses , as a decimal point.
* STRINGS - quote- or apostrophe-marked text. strings with no gaps between concatenate.
* BINARY BLOBS - strings marked as hexadecimal.
* TRUTHS - 0 is FALSE, anything else is TRUE.

## Symbols

| Symbol | Operation |
| --- | --- |
| \` | *Comment* <br> Everything from `` ` `` to the end of the line is ignored. |
| \\\<ID\> | *Label* <br> \<ID\> is a jump target. |
| ($ or $$)\<ID\> | *Variable* or *Static Variable* <br> \<ID\> is a variable. Static variables persist value between invocations, and are available globally via namespace. |
| (@ or @@)\<ID\> | *Function* or *Static Function* <br> \<ID\> is a function. All variable declarations on the stack prior are function variables. Static functions may only interact with provided function arguments or static variables. |
| \_ | *Address operator* <br> If preceeded by a DOT operator, gets the memory address of the \<ID\>. If preceeded by a LONG value (either hard-coded or in a variable), is a memory address. Addresses can be written to and read from, and the \[ operator may also be used. |
| \{ and \} | *Alias operators* <br> An identifier preceeding a closing } may be used to reference the stack back to {. This can be used to reference libraries via aliasing, or create lambdas. |
| \(\) | *NULL* <br> Signifies no value. |
| \#<value> | *Interpret as Hexadecimal* <br> This value is in hexadecimal. When applied to a string, the string becomes a binary blob. |
| ( and ) | *Negate* <br> Negates the final result of the expression between the braces. |
| \{\{ and \}\} | Embedded code (probably LLVM IR, tbd.) |

## Operators

Operators generally remove an item (or two) from the stack, apply the operation, then return the value to the stack.
Its important to note that the operation is applied _down_ - that is, the LHS is below the RHS in the stack.
If an operator is the last item in the stack, and the first item was a variable, the results are returned to that variable.
For example, `n 1 +` is equivalent to `n n 1 + =` or `n = n + 1` in conventional parlance.


| Operator | Operation |
| --- | --- |
| = | *Assignment* <br> Assignment can be to either a variable, function (as return value) or address. Assignment is by value copy only, so a deep copy requires a method. |
| + | *Addition* |
| - | *Subtraction* |
| * | *Multiplication* |
| % | *Modulus* |
| ~ | *Unary Complement* |
| & | *Binary AND* |
| \| | *Binary OR* |
| ^ | *BINARY XOR* |
| <- | *Shift Left* <br> Shifts 0's in from the right. |
| -> | *Shift Right* <br> Shifts 0's in from the left. |
| >> | *Signed Shift Right* <br> Shift whatever is in the MSB in from the left. |
| . | *DOT operator* <br> The DOT operator is a field reference. It can reference any named field in either a variable or a function. If used without context, refers to the local context. The DOT operator cannot be applied to an address. |
| \[ and \] | *Index Operator* <br> The index operator references numbered fields in a variable or method. If used without context, refers to the local context. If used on an address, applies an offset to that address. |
| ! | *Count Operator* <br> Returns the number of immediate child fields in the variable or function. Can be applied to the local context also. Cannot be used on an address. |
| \[\] | *Fields Operator* <br> Puts the contents of the referenced variable or function onto the stack. Cannot be used on an address. |

## Comparators

A comparator returns a true or false value. If the comparator is the last item in the stack, it then acts as a conditional - if the result is TRUE, the next line within scope is executed.
If it is false, and there is an ALT(`:`) immediately available in scope, that line is executed next.
If a comparator is not a conditional, it simply returns a truth value to the top of the stack.
It is necessary to assign a conditional to a variable - assignment is not automatically inferred the way it is with operators.
Only basic elements can be compared like this, and they are compared by value.

| Comparator | Operation |
| --- | --- |
| ? | *Equal To* |
| < | *Less Than* |
| ?< | *Less Than or Equal To* |
| > | *Greater Than* |
| ?> | *Greater Than or Equal To* |
| : | *Alternate* <br> This indictes an alternate execution path that will be executed if the preceeding conditional is 0. |

## Grammar

*TODO*

## Memory Model

*scandi*'s memory model consists of a table-based object store, and a working stack.
items are copied onto the stack, and copied back for assignment.

### Working Stack

Instructions push and pop from the stack.
The stack is cleared at the end of each line, regardless of state.

### Objects Table

The table-based object store starts from a root 'global' node, and variable, functions, etc. exist underneath this node.
Each node consists of two contexts - static and non-static.
Variables/functions exist within only one context - this is the difference between `$`,`@` and `$$`,`@@`.
static variables and functions may be accessed from any context.
static functions may only interact with static variables (plus their arguments, of course).

### Memory Mapping

This is determined by the ABI - *scandi* was intended to complement my 8-bit SIMD
multi-core CPU, and the exact layout of MMIO etc. is context specific.
*scandi* allows the mapping/dereferencing of objects to memory addresses, but not vice-versa.

## Standard Library

It is intended that *scandi* will ship with a small library enhancing its functionality via cstdlib.
This will be through built-in system calls, and it is not intended at this stage to allow programmatic
access elsewise (though this could be added as a stdlib feature in the future).
