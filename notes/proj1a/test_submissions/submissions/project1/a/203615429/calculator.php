<?php
if( $_GET['myInput'] )
{
	$input = $_GET['myInput'];
	
	$pattent = "/^[0-9]+([.][0-9]+)?([\+\-\*\/][0-9]+([.][0-9]+)?)*$/";
	
	if( preg_match( $pattent, $input ) )
	{
		eval("\$result = $input;");
		$result = $input." = ".$result;
	}
	else
	{
		$result = "Invalid input expression ".$input.".";
	}
}
?>
<html>
	<head>
		<title>CS 143 Project 1a</title>
	</head>
<body onload="Sunny.myInput.focus()">
	CS 143 Project 1a <br>
	Chi Kin Cheang ~ 203-615-429 <br>
	beef@ucla.edu <br>
	<form name="Sunny" action="<?=$_SERVER['PHP_SELF']?>" method="get">
		<input type="text" name="myInput">
		<input type="submit" value="Calculate">
	</form>
	<br>
<?php
if( $result )
{
	echo "Result<br>";
	echo $result;
}
?>
</body>
</html>