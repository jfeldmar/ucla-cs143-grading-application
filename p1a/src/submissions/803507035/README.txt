CS143 PROJECT 1A
Name:  Erick Romero
UID: 803-507-035
Email: erickrom@ucla.edu

Notes: The only known bug I could not fix was the case when two plus signs (+) were inputed in between numbers.  For example, the operation 2++2 will pass the regular expression test, but the 'eval' function will fail and output nothing, since I disabled error_reporting.  The - sign works fine, for example, the operation 2.3+-1.25 yields the correct answer 1.05.  Division by zero also doesn't print out the 'Invalid input expression' message, but it at least it handles those errors gracefully (withouth raw PHP error messages. 
