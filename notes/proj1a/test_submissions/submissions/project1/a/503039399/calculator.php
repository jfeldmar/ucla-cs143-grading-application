<html>
<body>
<h1>Calculator</h1>
Type an expression in the following box (e.g., 10.5+20*3/25).
<br>
<br>
<form action="." method="get">
<input type="text" name="calcInput" />

<input type="submit" value="Calculate" />
</form>
<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression.
<li>The evaluation follows the standard operator precedence.
<li>The calculator does not support parentheses. 
<li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
</ul>
<?php
function ValidateInput($input)
{
	if ($input == ".")
		return 0;
	if (preg_match("/\/0*\.?$|\/0*\.?0*[\*\/\+-]/i", $input) == 0)
	if (preg_match("/[\/\*+-]\.[\/\*\+-]|^[\/\*]|[\/\*\+-][^0-9\.]|[\/\*\+-]$|[^0-9\.\/\*\+-]|[\/\*\+-]\.$|^\.[\/\*\+-]|\.[0-9]*\./i", $input) == 0)
		return 1;
	else
		return 0;
}

function GenerateRandomString($max)
{
	$length = mt_rand(1,$max);
	$outputString = "";
	
	while(strlen($outputString)<$length)
	{
	$choice = mt_rand(0,15);
	
	switch ($choice)
	{
	case 0:
		$currentChar = '0';
		break;
	case 1:
		$currentChar = '1';
		break;  
	case 2:
		$currentChar = '2';
		break;
	case 3:
		$currentChar = '3';
	  break;  
	case 4:
		$currentChar = '4';
	  break;
	case 5:
		$currentChar = '5';
	  break;  
	case 6:
		$currentChar = '6';
	  break;
	case 7:
		$currentChar = '7';
	  break;  
	case 8:
		$currentChar = '8';
	  break;
	case 9:
		$currentChar = '9';
	  break;  
	case 10:
		$currentChar = '.';
	  break;
	case 11:
			$currentChar = '+';
	  break;  
	case 12:
		$currentChar = '-';
	  break;
	case 13:
		$currentChar = '/';
	  break;  
	case 14:
		$currentChar = '*';
		break;
	default:

	}
	$outputString = $outputString . $currentChar;
}

return $outputString;	
}


function BuildCalcArgString($input)
{
	$calcArg = "\$result = $input;";
	return $calcArg;
}

function testParser($numTests)
{
	for ($i=0; $i<$numTests; $i++)
	{	
		$input = GenerateRandomString(20);
		echo "<p>" . $input . " : ";
		if ($input != NULL)
		{
			if (ValidateInput($input))
			{
				$calcArg = BuildCalcArgString($input);
				eval($calcArg);
				echo $input . " = " . $result;

			}
			else
			{
				echo "Invalid input expression " . $input . ".";
			}
		}
	}
	
	
	
}
$input = $_GET["calcInput"];
if ($input != NULL)
{
	if (ValidateInput($input))
	{
		$calcArg = BuildCalcArgString($input);
		eval($calcArg);
		echo "<h2>Result</h2>";
		echo $input . " = " . $result;
		
	}
	else
	{
		echo "<h2>Result</h2>";
		echo "Invalid input expression " . $input . ".";
	}
}
//testParser(10000);

?>
</body>
</html>