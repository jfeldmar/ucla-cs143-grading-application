<html>
<head><title>Calculator</title></head>
<body>

<h1>Calculator</h1>

Type an expression in the following box(e.g., 10.5+20*3/25).
<br>
<br>

<form action="calculator.php" method="get">
<input type="text" name="equation" value="" size="20">
<input type="submit" value="Calculate">
</form>

<ul>
  <li>Only numbers and +,-,* and / operators are allowed in the expression.</li>
  <li>The evaluation allows the standard operator precedence.</li>
  <li>The calculator does not support parentheses.</li>
  <li>The calculator handles invalid input "gracefully". It does not output PHP error messages.</li>
</ul>



<?php

$equation = $_GET["equation"];

echo "<h2> Result </h2>";

if ( !empty($equation) ){
	$temp = 0;

	if ( 0 == preg_match('/^[-0-9+*\/\s\.]{1,}$/', $equation) )
		echo "Invalid input expression $equation";
	else if ( 1 == preg_match('/[-+*\/][-+*\/]/', $equation) )
		echo "Invalid input expression $equation";
	else if ( eval("\$temp = $equation;") === FALSE)
		echo "Invalid input expression $equation";
	else if ( $equation ){
		echo "$equation = ";
		eval ("\$equation = $equation;");
		echo "$equation";
	}
	else
		echo "Invalid input expression $equation";
}
?>



</body>
</html>
