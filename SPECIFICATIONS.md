# SPECIFICATIONS

================================

## Keywords
Should support standard "C-Style" language keywords like:

`if, else, for, while, struct, private, public, true, false, return`

In addition, it should have the following keywords
`proc, byval, discard, persistent`

* `proc` - Declares function: `proc name(int var1, string var2){}`
* `byval` - To be used in function calls, passes object in by value not by address. This creates a copy in the function's
scope. Ex: `func(byval variable_name);` would (deep)copy the value of variable_name into `func()`
* `discard` - Moves the inputted variable into the function's scope so it is deleted after the function exits.
Ex: `func(discard variable_name)` would delete variable_name after `func()` is done executing
* `persistent` - Creates a variable without an associated scope ID so they're not garbage collected until explict
deletion.

## Literals
* Identifiers - names for variables
* Strings - Text literals represented by "{text}"
* Floats - Represented by any number followed by `f`
* Doubles - Represented by any number followed by `d`
* Longs - Represented by any whole number followed by `l`
* ulong64 - Represented by any whole number followed by `ull`
* Ints - Default for any whole number literal can also be specified by `i`

Example:
`double` name = `3.14d` : This should create a double with value 3.14 under the name 'name'

## Operands
* `and` or `&&` - And
* `or` or `||` - Or
* `xor` - Exclusive or
* `(` - open paren
* `)` - close paren
* `{` - start code block
* `}` - end code block
* `;` - end sentence (newlines should be ignored for tokenizer)
* `!` or `not` - Not operator
* `!=` - Not equal operator
* `^` - exponent operator
* `[` - array start operator
* `]` - array end operator
* `.` - dot operator - accesses members of identifiers
* `+` - addition operator
* `-` - subtraction operator
* `*` - multiplication operator
* `/` - division operator
* `/i`- floor division operator
* `>` - greater than operator
* `>=` - greater than equal to operator
* `<` - less than operator
* `<=` - less than equal to operator
* `&` - get address (unsigned long long)
* `=` - set operator
* `==` - equal operator (byval)
* `===` - abs-equality operator (checks memory addresses)