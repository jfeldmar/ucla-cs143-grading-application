<html>
<body>

<h1>Calculator</h1>
<p>
Type an expression in the following box (e.g., 10.5+20*3/25). 
</p>

<form method="POST">
<input type="text" name="Input" />
<input type="submit" value="Calculate" />
</form>

<ul>
	<li>Only numbers and +,-,* and / operators are allowed in the expression.
	<li>The evaluation follows the standard operator precedence.
	<li>The calculator does not support parentheses.
	<li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
</ul>
Here are some(but not limit to) reasonable test cases:

<ol>	
	<li>	A basic arithmetic operation: 3+4*5=23
	<li>	An expression with floating point or negative sign : -3.2+2*4-1/3 = 4.46666666667, 3+-2.1*2 = -1.2
	<li>	Some typos inside operation (e.g. alphabetic letter): Invalid input expression 2d4+1
</ol>



<?php

$str = $_POST["Input"];

$valid_op = '^([0-9]+[+*-/])*[0-9]+$';

$num = '[-]?[0-9]+';
$dec = '[-]?[0-9]+[.]?[0-9]+';
$op = '[*\-\+\/]';

$val_op2 = "/^(($num|$dec)$op)*($num|$dec)$/";

//if string is null, do nothing
if($str=="")
{

}
else if(!preg_match($val_op2, $str))
{
	echo "<H2> Result</H2>";
	echo "Invalid input expression $str";
}
else
{

//calculate the value from the input string
echo "<H2> Result</H2>";
echo "$str = ";

eval("\$str = $str;");
echo $str;

}
?>

</body>
</html>
