<html>
<body>

<h1>Calculator</h1>

<form action="calculator.php" method="get">
<input type="text" name="input" />
<input type="submit" value="Solve" />
</form>

<?php
// get the input from the form into $var1
$var1 = $_GET["input"];

// this was added to not display an error when the page is opened
if ($var1 == NULL)
{
	// do nothing
}
// if there are any letters, or special symbols
else if ( (preg_match( "/(.*)([A-Za-z]+)(.*)/", $var1)) ||
	      (preg_match( "/[!@#%^`~&_=:;<>?|]/", $var1))
		)
{
	// display error message
	echo "There seems to be a problem with your input.\n";
}
// if the input is a valid math expression
else if (preg_match( "/(\ *)(((\d+)(\.?)(\d*)(\ *)[\+\-\/\*])*)(\ *)(\d+)(\.?)(\d*)(\ *)/", $var1))	 
{
	// evaluate the input and print it
	eval("echo $var1;");
}
// if the input is invalid somehow
else
{
	echo "There seems to be a problem with your input.\n";
}
?>
	
</body>
</html>
