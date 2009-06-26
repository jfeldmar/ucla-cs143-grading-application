Jeffrey Su
703449121
jeffreysu@ucla.edu

This program catches divide by 0 and returns Undefined if the expression contains divide by 0.
Otherwise, it returns the result returned by eval and attempted to follow the initial demonstration program as closely as possible.
Octal numbers do not receive any special handling.  They go straight to eval.
Some differences from the version as of 10-7-08:
1++1 is Invalid instead of error.
1--1 is Invalid instead of error.
1*+1 is 1 instead of Invalid.  Same with divide.
0/0 is Invalid instead of error.

My definition of a valid expression is as follows:
It must begin with a number.
	Numbers may contain digits from 0-9 and up to one period.
It may also include an operator and an operand.
	The operator may be +, -, *, or /.
	The operand must be a number.
Any number of operator/operand pairs may follow.
In addition...
	No spaces are allowed.
	No parenthesis are allowed.

Additional rules added later...
A + or - may occur at the beginning.
A + or - may occur after any operator.
++ anywhere is invalid.
-- anywhere is invalid.

Some notable test cases
No expression = blank output.
0/0 = Undefined
1+3/0 = Undefined
+1 = 1
-1 = -1
.1 = Invalid input expression
Octal numbers return unusual results.
Hex numbers = Invalid input expression.
1,234 = Invalid input expression.
1E10 = Invalid input expression.
pi = Invalid input expresson.
1+-1 = 0
1-+1 = 0
1*-1 = -1
1/-1 = -1
1/01+1 = 2
10/01 = 10
1. = 1
1+3/0000 = Undefined
html code = The result of dispaying html code.
a lot of 1's = Inf
