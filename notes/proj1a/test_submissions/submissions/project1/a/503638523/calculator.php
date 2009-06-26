﻿<html>
<head><title>Calculator</title></head>
<body>

<h1>Calculator</h1>
Type an expression in the following box (e.g., 10.5+20*3/25). 
<p>
<form action="calculator.php" method="GET">
<input type="text" name="expr">
<input type="submit" value="Calculate">
</form>
</p>
<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression.
<li>The evaluation follows the standard operator precedence.
<li>The calculator does not support parentheses. 
<li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
</ul>

<?php

$string = $_GET["expr"];
if($string)
{
echo "<h2>Result</h2>";

if(!preg_match("/^((\-|)(\d+|\d+\.\d*)|(\-|)(\d+|\d+\.\d*)((\+|\-|\*|\/)(\-|)(\d+|\d+\.\d*)+)+)$/",$string))
{
	echo "Invalid input expression ".$string;
}
elseif(preg_match("/(\/)(0+|0+\.0+)/",$string))
{
	echo $string." = undefined. (cannot divide by zero.)";
}
elseif(preg_match("/\-\-/",$string))
{
	$str2 = preg_replace("/\-\-/", "+", $string);
	eval("\$ans = $str2;");
	echo $string." = ".$str2." = ".$ans;
}
else{
	eval("\$ans = $string;");
	echo $string." = ".$ans;
}
}
?>

</body>
</html>