<html>
<body>

<form action="calculator.php" method="post">
Expression: <input type="text" name="exp" />
<input type="submit" />
</form>

<?php
if ($equ = $_POST["exp"]) { // to make sure exp is not empty
	if (preg_match( "/^((([-]?([0-9]*\.?[0-9]+|[0-9]+))((\+|\-|\/|\*)[-]?([0-9]*\.?[0-9]+|[0-9]+))*))$/", $equ)) {
		eval("\$ans = $equ ;");
		echo "Answer = ".$ans;
	}
	else {
		echo $equ . " contains invalid characters. Please re-input expression.";
		exit;
	}
}
else {
	echo "Please input an expression.";
}
?>

<br><br>
Jonathan B. Lee <br>
603445374

</body>
</html>