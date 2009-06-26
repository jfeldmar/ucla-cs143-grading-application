<html>
<body>

<h1>Calculator</h1>

<p>Type an expression in the following box (e.g., 10.5+20*3/25).</p>

<p>
<form action="calculator.php" method="GET">
<input type="text" name="expr"><input type="submit" value="Calculate">
</form>
</p>

<ul>
  <li>Only numbers and +,-,* and / operators are allowed in the expression.
  <li>The evaluation follows the standard operator precedence.
  <li>The calculator does not support parentheses. 
  <li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
</ul>
<p> Here are some(but not limit to) reasonable test cases: </p>
<ol>
  <li> A basic arithmetic operation:  3+4*5=23 </li>
  <li> An expression with floating point or negative sign : -3.2+2*4-1/3 = 4.46666666667, 3+-2.1*2 = -1.2 </li>
  <li> Some typos inside operation (e.g. alphabetic letter): Invalid input expression 2d4+1 </li>
</ol>


<?php

//The expression that's read in
$expr = $_GET["expr"];
//The regular expression we want
$pattern = "/^[0-9\.+*-\/]{1,}$/";
//Whether or not preg_match matches 0 = false 1= true
$match = preg_match($pattern, $expr);

//If no expression, output nothing
if (!$expr) {}
//Else if the expression is invalid
else if (!$match) {
	echo "<h2>Result</h2>";
  echo "<p>Invalid input expression " . $expr . ".</p>";	
}
//Otherwise, output the results
else if ($expr) {
  echo "<h2>Result</h2>";
  eval("\$eval = $expr;");
  echo "<p>" . $expr . " = " . $eval . "</p>";
}

?>

</body>
</html>
