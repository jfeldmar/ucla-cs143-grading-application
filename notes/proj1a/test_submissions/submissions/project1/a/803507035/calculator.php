<HTML>
<BODY>
<H1>Calculator</H1>

<form action="calculator.php" method="get">
<input type="text" name="u_input" />
<input type="submit" value="Calculate" />
</form>

<H2>Result</H2>
<?php
	error_reporting(0);
	if($_GET["u_input"])
	{
		$input=$_GET["u_input"];
		$errors = 0;
		
		//Check that it doesn't end on an operation sign
		if(preg_match('/[\+\-\*\/]$/', $input) == 1)
		{
			$errors++;
			//echo "Invalid input expression (1) $input";
		}
		//Check for case with 2 or more decimal points
		if(preg_match('/^[0-9]*[.]+[0-9]*[.]+[0-9]*$/', $input) == 1)
		{
			$errors++;
			//echo "Invalid input expression (2) $input";
		}
		//else
		//Main valid form
		//$valid = "/^[\\+\\-]?[.]?[0-9]+[.]?[0-9]*[\\+\\-\\*\\/0-9]*[.]?[0-9]*$/";
		$valid = "/^([\+\-]?([0-9]*.?)?[0-9]+)+[\+\-\*\/]?$/";
		if(preg_match($valid, $input)> 0 && $errors == 0)
		{
			eval("\$result=$input;");
			echo $input . " = " . $result;
		}
		else 
		{
			echo "Invalid input expression $input";
			//echo "Errors is $errors";
		}		
		//}	
	}
?>
	
</BODY>
</HTML>