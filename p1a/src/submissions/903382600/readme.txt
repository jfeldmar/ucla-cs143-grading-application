README FOR PROJECT 1A

Name: Kenny Wong
SID: 903382600
Email: nakoxathena@hotmail.com

Function: Calculator
Input: A valid mathematical expression
Output: The answer to the expression

As per the instructions, this is a simple calculator done in PHP.  It supports integers, decimals, as well as negative numbers.  Parenthesis are not supported.  Valid operations are +, -, /, *. Whitespace is not supported and will return an error.

The code consists of a simple text form for the user to input the expression.  Error checking is next performed via regular expressions.  The regular expressions can be seen within the code.  If the expression is not valid, an error message is returned.

If the expression is deemed valid, another regular expression check is run to determine if there is division by 0.  If the expression is not valid, the answer variable is set to blank.  Otherwise the expression is evaluated and stored into the answer variable.  The answer is the outputted to the user in the form $Expression = $Answer.

