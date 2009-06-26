<html>
<head>
<title>Edward Chang's PHP Calculator</title>
</head>
<body>
<h1><font face=verdana>Edward Chang's Calculator</h1>
<font face=verdana size=2>Please type an expression in the following box (e.g. 8+8*8-8/8)<p>

<form action="calculator.php" method="post">
<input type="text" name="calcstring" style="width:250px" />
<input type="submit" value="Calculate" />
</form>


<p>
Some guidelines:<br>
<b>#</b> Only numbers and +,-,* and / operators are allowed in the expression.<br>
<b>#</b> The evaluation follows the standard operator precedence.<br>
<b>#</b> The calculator does not support parentheses.<br>
<b>#</b> The calculator handles invalid input "gracefully". It does not output PHP error messages
<p>

<?php
	$calcstring2 = $_POST["calcstring"];

	if ($calcstring2 != "")
	{
		echo "<h3>Result</h3>";
		$dbzero1 = "/.*\/0.*/";

		$valid = "/^[0-9]*\.?[0-9]*([-+*\/]{1}[0-9][0-9]*\.?[0-9]*)*$/";
		if (preg_match($dbzero1,$calcstring2,$matches) == 1)
		{
			echo "Invalid input expression. ".$calcstring2."  Please try again.";
		}
		/*
		else if (preg_match($dbzero2,$calcstring2,$matches) == 1)
		{
			echo "Invalid input expression. ".$calcstring2."  Please try again.";
		}
		*/
		else if (preg_match($valid,$calcstring2,$matches) == 1)
		{
			eval("\$ans = $calcstring2 ;");
			echo $ans;		
		}
		else
			echo "Invalid input expression. ".$calcstring2."  Please try again.";
	}

/*
	if ($calcstring2 != "")
	{
		eval("\$ans = $calcstring2 ;");
		echo "<h3>Result</h3>";
		if (floatval($ans) == FALSE)
			echo "Invalid input expression.  Please try again.";
		else
		{
			echo $ans." = ".$ans;
		}
		
	}
*/
?>

</body>
</html>