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
			if ($expr != "")
			{
				$tab = "\t";
				$endl = "\n";

				echo "<h2>Result</h2>".$endl;
				
				$pattern = "/^[+\-]?[0-9][0-9]*\.?[0-9]*([+\-*\/][+\-]?[0-9][0-9]*\.?[0-9]*)*$/";
				$valid = preg_match($pattern, $expr);
					
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
				if ($valid && preg_match("/--/", $expr) == 0 && preg_match("/\+\+/", $expr) == 0)
				{
					$pattern1 = "/\/0+(\.0*)?$/";
					$pattern2 = "/\/0+(\.0*)?[+\-*\/]/";
					$divideByZero = preg_match($pattern1, $expr) + preg_match($pattern2, $expr);
					if ($divideByZero)
						$result="Undefined";
					else
						eval('$result='."$expr;");
					if (strval($result)=="")
						$result = "Undefined";
					echo $tab.$expr." = ".$result.$endl;
				}
				else
				{
					//Display error message
					echo $tab."Invalid input expression ".$expr.".".$endl;
				}
			}
		}
	?>
</body>
</html>