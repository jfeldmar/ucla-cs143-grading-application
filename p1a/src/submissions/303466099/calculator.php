<html>
<head>
<title>Calculator</title>
</head>
<body>

<h1>Calculator</h1>

<form action="calculator.php" method="get">
<input type="text" name="input" />
<input type="submit" value="Calculate"/>
</form>

<h2>Result</h2>

<?php

$input = $_GET["input"];

$error = 0;

$expr = "/[^0-9.+\-*\/]/";
if(preg_match($expr,$input))
	echo "Invalid input expression $input";

else
{
	// Multiplication
	$expr = "/(-?[0-9]+\.?[0-9]*\*-?[0-9]+\.?[0-9]*)/";
	while(preg_match($expr,$input,$matches))
	{
		eval("\$ans = $matches[0];");
		$input = preg_replace($expr,$ans,$input,1);
	}

	// Division
	$divbyzero = 0;
	$expr = "/(-?[0-9]+\.?[0-9]*)\/(-?[0-9]+\.?[0-9]*)/";
	while(preg_match($expr,$input,$matches) && $divbyzero == 0)
	{
		if($matches[2] == 0)
		{
			$divbyzero = 1;
		}
		else
		{
			eval("\$ans = $matches[1] / $matches[2];");
			$input = preg_replace($expr,$ans,$input,1);
		}
	}

	// Addition
	$expr = "/(-?[0-9]+\.?[0-9]*\+-?[0-9]+\.?[0-9]*)/";
	while(preg_match($expr,$input,$matches) && $divbyzero == 0)
	{
		eval("\$ans = $matches[0];");
		$input = preg_replace($expr,$ans,$input,1);
	}

	// Subtraction
	$expr = "/(-?[0-9]+\.?[0-9]*)\-(-?[0-9]+\.?[0-9]*)/";
	while(preg_match($expr,$input,$matches) && $divbyzero == 0)
	{
		eval("\$ans = $matches[1] - $matches[2];");
		$input = preg_replace($expr,$ans,$input,1);
	}
	
	if($divbyzero == 0)
		echo $_GET["input"] . " = $ans";
	else
		echo "Cannot divide by 0";

}

?>

</body>
</html>
