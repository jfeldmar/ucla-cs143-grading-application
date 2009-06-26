<html>
<body>
<form action="calculator.php" method="GET">
Insert Expression Here: <input type="text" name="exp" />
<input type="submit" name "Enter"/>
</form>
<?php
if($_GET["exp"]){
	$equ = $_GET["exp"];
	$regexp = "/^-?(\d+(\.\d+)?)([-+*\/](\d+(\.\d+)?))*$/";
	if (preg_match($regexp,$equ)) {
		echo $equ;
		eval("\$ans = $equ;");
		echo " = ".$ans."<br/>";
	}
        else {
		echo $equ." is invalid expression";
	}
}
?>
</body>
</html>
