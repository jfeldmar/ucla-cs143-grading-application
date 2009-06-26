<html>
<head><title>Calculator</title></head>
<body>

<h1>Calculator</h1>
Type an expression in the following box (e.g., 10.5+20*3/25). 
<p>
	<form action="calculator.php" method="GET">
	<input type="text" name="expr"><input type="submit" value="Calculate">
	</form>
</p>
<ul>
	<li>Only numbers and +,-,* and / operators are allowed in the expression.
	<li>The evaluation follows the standard operator precedence.
	<li>The calculator does not support parentheses. 
	<li>The calculator handles invalid input "gracefully". 
		It does not output PHP error messages.
</ul>

<?php
// get the first element in the list and see that it's a floating-point or 
// integer number.
// issue: this allows the string to be of form "." because each \d can be 
//        present 0 or more times, which means "." is valid, but this 
//        case is taken care of later
eval("\$str = \$_GET[\"expr\"];");
$isvalid = preg_match("/-?\d*(\.\d*)?/", $str, $firstnum, PREG_OFFSET_CAPTURE);
$index = strlen($firstnum[0][0]);
//print_r($firstnum[0]);

if($isvalid == 0) 	// if the first input isn't actually a number
{	
	echo "<h2>Result</h2>";
	echo "Invalid input expression $str.";
}
else			// we're safe, so keep going
{
	$inp_len = strlen($str);
	// echo "input length: $inp_len<br />";
	  // go through the length of the string and if preg_match returns 0:
	  //	1) reached end of some valid string
	  //	2) reached some sort of malformed part of input
	while($index < $inp_len && $isvalid == 1)
	{
		  // force the operator to be checked
		$isvalid = preg_match("/[-+\/*]-?\d+(\.\d*)?/", $str, $words, 
				PREG_OFFSET_CAPTURE, $index);

		if($isvalid == 1)	// got a regex match, so move index up
		{
		//	print_r($words);
			// echo "index: $index <br />";
		//	echo "<br />";
			$index += strlen($words[0][0]);
			// echo strlen($words[0][0]);
		}
	}

	//echo $index;
	  // if the string is valid and it's not the case "."
	if($index >= $inp_len && strcmp($firstnum[0][0], ".") != 0)
	{
		$works = eval("\$res = $str;");		// calculate the valid expression
		if($works == 0)
			ob_get_flush();
		echo "<h2>Result</h2>";
		echo $str . " = ";
		echo $res;
	}
	else
	{
		echo "<h2>Result</h2>";
		echo "Invalid input expression $str.";
	}
}
?>

</body>
</html>
