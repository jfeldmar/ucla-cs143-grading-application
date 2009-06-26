<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" 
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">

<head>
<title>Calculator</title>
</head>

<body>

<h1>Calculator</h1>
<p>Type an expression in the following box (e.g. 80/2+1*2):</p>
<form action="calculator.php" method="get">
	<p>
		<input type="text" name="expr" />
		<input type="submit" value="Calculate" />
	</p>
</form>
<ul>
	<li>Only numbers are +-*/ are allowed in the expression.</li>
	<li>Evaluation follows standard operator precedence.</li>
	<li>Calculator does not support parentheses.</li>
	<li>Calculator handles invalid input "gracefully": no ugly PHP errors!</li>
</ul>

<?php

	if (($expr = $_GET["expr"]) != NULL) {

		echo "<h2>Result</h2>";

		if (preg_match("/[^\d\.\+\*\-\/]/",$expr) || // If any character except 0-9 or +*/- is in the expression
		    !preg_match("/^-?\.?\d/",$expr)      || // Or the first part of the expression is not a number
		    !preg_match("/\d\.?$/",$expr)        || // Or the last part of the expression is not a number
		    preg_match("/([\+\*\/\-][\+\*\/]|[\+\*\/\-]{3,}|--)/",$expr) ||
		    	// Or we have two consecutive operands (except for +-, *-, and /-) or three consecutive operands
		    preg_match("/\D\.\D/",$expr) || // Or a lone decimal point (with no associated number)
		    preg_match("/\.\d*\./",$expr) || // Or two decimal points in the same number
		    preg_match("/\/[0.]+([^\d.]|$)/",$expr)) // Or division by zero
		{
			echo "Invalid input expression $expr."; // Output an error message
		}

		else  {
			eval('$result = '."$expr;");
			echo $expr.' = '.$result;
		}
	}
?>

</body>
</html>
