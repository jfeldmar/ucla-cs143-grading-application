README for Project 1A
Peter A. H. Peterson
203462520
pedro@tastytronic.net

My calculator borrowed all the text and functionality from the course calculator and is as faithful a copy as I could make it with a few enhancements. (See ENHANCEMENTS, below.)

OVERVIEW

The internals are simple; any input string is validated first to eliminate bad input. Then, some changes are made to make a certain kind of bad input acceptable. Finally, the entire string is evaluated with eval() and the result is returned into a variable. However, if the eval() fails, there will be no output, thanks to the @ operator which precedes the eval(). If there was not an error, and if the result is a numeric string, it is printed.

INPUT VALIDATION

My input validation occurs in several stages:

1. The input must contain at least one number.
2. The input must only contain the following characters: -, *, /, +, digits, and spaces.
3. No "double operators" are allowed from the operations *+/. (e.g., ** or *+)
4. An operator *+/ followed by a - (minus) is allowed (negative operand), but an operator +*/ followed by two or more minuses (e.g., *--) is not.
5. A double minus is allowed (--, subtraction of a negative), but three or more minuses in a row is not allowed (---).

If any of these rules are broken, it is an invalid input.

INPUT REPAIR

A "--" is valid (subtraction of a negative value), however eval() does not evaluate it properly without a space in between the minuses. As a result, any "--" string is replaced with a "- -" string, which eval() does handle correctly. Since "---" is not allowed, this will not cause a problem. 

If there is not a "--" string in the operation, nothing happens.

EVALUATION

At this point, the operation string is evaluated and the result is stored in a variable. As a protection against parse errors, the eval() statement is protected with a @ character which supresses the error message. A parse or eval failure leaves a null character in the result.

If the result includes at least one digit, it is printed. Otherwise it is considered invalid input.

ENHANCEMENTS

My calculator.php features some ehnancements over the example code.

1. My code (thanks to the "--" expansion), handles the "1--1" scenario properly. The string is expanded into "1- -1", which is properly evaluated.

2. My code correctly identifies "1---1" as an invalid expression, rather than printing "1--1 = ".

3. My code gives an invalid expression for divide by zero errors, rather than just printing "1/0 = ".
