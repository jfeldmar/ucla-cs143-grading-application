<html>
<head>
<title>Calculator</title>
</head>

<body>
	<h1>Calculator</h1>
	<p>Type an expression in the following box (e.g., 10.5+20*3/25).</p>

	<form action="calculator.php" method="GET">
		<input type="text" name="expr"/><input type="submit" value="Calculate"/>
	</form>
	<ul>
		<li>Only numbers and +,-,* and / operators are allowed in the expression.
		<li>The evaluation follows the standard operator precedence.
		<li>The calculator does not support parentheses. 

		<li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
	</ul>

	<?php
		if (sizeof($_GET) > 0 && array_key_exists("expr", $_GET))
		{
			$expr = $_GET["expr"];
			if (sizeof($expr) > 0)
			{
				$tab = "\t";
				$br = "<br>";
				$endl = "\n";

				echo $tab."<h2>Result</h2>".$endl;
				
				$valid = true;
				//TODO:
				/*
				To determine if an expression is valid...
				It must begin with a number.
					Numbers may contain digits from 0-9 and up to one period.
				It may also include an operator and an operand.
					The operator may be +, -, *, or /.
					The operand must be a number.
				Any number of operator/operand pairs may follow.
				In addition...
					No spaces are allowed.
					No parenthesis are allowed.
				*/
				if (valid)
				{
					//evaluate expression
					echo $tab."<p>".$expr." = </p>".$endl;
				}
				else
				{
					//TODO:
					//display error message
				}
			}
		}
	?>
</body>
</html>

