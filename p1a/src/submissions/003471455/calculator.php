<html>
<head>
	<title>Richard's Calculator</title>
</head>
<body>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="GET">
Input:&nbsp;<input type="text" name="input"/>
<input type="submit"/>
</form>
<?php 
	$calculate = $_GET["input"];
	$isValid = true;
	$errorMsg = "";
	$pattern = '/\/0([^\.]|$)/';
	preg_match($pattern,$calculate,$matches);
	if($matches[0] != "") {
		$isValid = false;
		$errorMsg = "Division by 0";
	}
	$pattern = '/(((\-?\d+\.?\d*[\*\+\-\/])*)((\-?\d+\.?\d*){1,1}))$/';
	preg_match($pattern,$calculate,$matches);
	if($matches[0] != $calculate) {
		$isValid = false;
		$errorMsg = "Input is invalid!";
	}
	if($isValid == true && $calculate != "") {
		eval("\$ans = $calculate;");
		echo "$calculate = ".$ans;
	}
	echo $errorMsg;
?>
</body>
</html>