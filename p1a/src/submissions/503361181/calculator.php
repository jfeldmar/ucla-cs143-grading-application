<html>
<head><h1>Calculator</h1></head>
<body>
<form action="calculator.php" method="post">
Enter an Expression: <input type="text" name="expr" />
<input type="submit" />
</form>
<?php
if(isset($_POST["expr"]))
{
	$valid = "/[-]?\d[.]?\d*([*+-\/]\d[.]?\d*)*/";
	$divbyzero = "/\/0/";
	$eqn = $_POST["expr"];
	if(preg_match($valid, $eqn, $matches) && $matches[0] == $eqn)
		if(preg_match($divbyzero, $eqn))
			echo "Division by zero.";
		else
		{
			eval("\$ans = $eqn ;");
			echo $eqn ." = ". $ans;
		}
	else
		echo "Invalid input.";
}
?>
</body>
<html>
