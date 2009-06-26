<html><head><title>Calculator</title></head>
<body>
<h1>Calculator</h1>

<p>Type an expression in the following box (e.g., 10.5+20*3/25). </p>

<form method="get" name="calc">
<p>
	<input type="text" name='ops' value=""><input type="submit" name='submit' value="Calculate">
</p>
<p>
	<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression.</li>
<li>The evaluation follows the standard operator precedence.</li>
<li>The calculator does not support parentheses.</li>
<li>The calculator handles invalid input "gracefully". It does not output PHP error messages. </li>
</ul>
<p>
Here are some(but not limit to) reasonable test cases: 
</p>

<ol>
<li>A basic arithmetic operation: 3+4*5=23</li>
<li>An expression with floating point or negative sign : -3.2+2*4-1/3 = 4.46666666667, 3+-2.1*2 = -1.2</li>
<li>Some typos inside operation (e.g. alphabetic letter): Invalid input expression 2d4+1</li>
</ol>
</form>


<?php

$ops = $_REQUEST['ops'];
if ($ops != "") {

	echo "<h2>Result</h2>";
	if (!preg_match("/\d/", $ops) || # must contain 1 digit
		preg_match("/[^- \d+*\/.]+/", $ops) || # match bad chars
		preg_match("/[+*\/][+*\/]+/", $ops) || # double ops
		preg_match("/[+*\/]--+/", $ops) || # op + >2 minuses
		preg_match("/---+/", $ops)  # 3> minuses alone
		
		) {

		echo "Invalid input expression $ops.";

	} else {
		# fix -- by expansion to "- -"
		#echo "<p>$ops</p>";
		$ops = preg_replace('/--/', '- -', $ops);

		# make result eval line
		$result = "\$result = " . $ops . "; return \$result;";
		#echo $result;
		@eval($result);
		if (preg_match("/^-?\d+(\.\d+)?/", $result)) {
			echo "<p>$ops = $result</p>";
		} else {
			echo "Invalid input expression $ops.";
		}
	}

}


?>

</body>
</html>
