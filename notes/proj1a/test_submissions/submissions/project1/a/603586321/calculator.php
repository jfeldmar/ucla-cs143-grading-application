<html>
	<head>
		<title>Calculator</title>
	</head>
	<body>
		<h1>Calculator</h1>
		Type an expression in the following box (e.g., 10.5+20*3/25). 
		<p>
			<form method="GET">
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
		Here are some(but not limit to) reasonable test cases:
		<ol>
  			<li> A basic arithmetic operation:  3+4*5=23 </li>
  			<li> An expression with floating point or negative sign : -3.2+2*4-1/3 = 4.46666666667, 3+-2.1*2 = -1.2 </li>
  			<li> Some typos inside operation (e.g. alphabetic letter): Invalid input expression 2d4+1 </li>
		</ol>
		<?php
			if($_GET["expr"]) {
				echo "<h2>Result</h2>";
				// Remove white space from expression
				$expr = str_replace(' ', '', $_GET["expr"]);
				// Regular expression for numbers
				$num_re = '-?\d+(\.\d+)?';
				// Regular expression for operators
				$oper_re = '[\/+\-*]';
				// Regular expression for a valid expression
				$expr_re = $num_re.'('.$oper_re.$num_re.')*';
				// Check if the expression is valid
				preg_match('/^'.$expr_re.'$/', $expr, $matches);
				if($matches[0])	{
					// If expression is valid ignore divide by zero error and output result
					@eval("\$ans = $expr;");
					echo "$expr = $ans";
				} else {
					// If expression is not valid output error message
					echo "Invalid input expression $expr.";
				}
			}
		?>
	</body>
</html>

