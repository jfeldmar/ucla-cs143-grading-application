<html>
<head><title>Calculator</title></head>
<body>

<h1>Calculator</h1>
Type an expression in the following box (e.g., 10.5+20*3/25).
<p>
<form action="" method="GET">
<input type="text" name="expr"><input type="submit" value="Calculate">
</form>
</p>
<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression.
<li>The evaluation follows the standard operator precedence.
<li>The calculator does noSt support parentheses.

<li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
</ul>

<?php
	// Verify that a submission has been made
	if ( isset($_GET['expr']) && !empty($_GET['expr'])) {
		// Output the title
		echo '<h2>Result</h2>';

		// Get the input
		$expr = trim($_GET['expr']);

		// Verify that the expression is in the correct format
		if  (
				@preg_match('/[^0123456789.*+-\/]/', $expr) || // Check: The input is limited to these.
				@preg_match('/[\*\/\+]$/',           $expr) || // Check: The end cannot be an operation.
				@preg_match('/^[\*\/\+]/',           $expr)    // Check: The start cannot be an operation.
			) {
			echo 'Invalid input expression ' . $expr . '.';
			exit();
		}

		// Evaluate the expression
		@eval("\$ans = $expr;");

		echo $expr . ' = ' . $ans;
	}
?>
</body>
</html>
