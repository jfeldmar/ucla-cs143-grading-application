Javier Rey
603586321
Jrey2010@ucla.edu

	The first thing I did was to get the expression passed in and remove all whitespace. I defined the regular expression for a number to be an optional negative sign followed by 1 or more digits with an option decimal point followed by 1 or more digits. I defined the regular expression for a mathematical expression as a number followed by zero or more operator/number pairs. If the expression passed in was valid I used the eval function on it and displayed the results (ignoring errors for the case of divide by zero). Otherwise I displayed an invalid expression message.

