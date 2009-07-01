<html>

<head>
<?php $title = "CALCULATOR" ?>
<title><?php print "$title"; ?></title>
</head>

<body bgcolor=white>
<h1><?php print "$title"; ?></h1>

<?php print "Type an expression in the following box (e.g., 10.5+20*3/25)." ?>
<br /> <br />

<form method = "POST" action = "calculator.php">
<input type="text" name="expression" />
<input type="submit" name="calculate" value="Calculate"/>
</form>

<?php
// Enable assert
assert_options (ASSERT_ACTIVE, 1);
// Turn off warnings for assert
assert_options (ASSERT_WARNING, 0);
// DO NOT display errors in the asserted code
assert_options (ASSERT_QUIET_EVAL, 1);

$exp = $_POST["expression"];
if($exp) {	
	$pattern = "/^\-?[1-9]\d*\.?\d*(([\+\-\*\/][1-9]\d*\.?\d*)|([\+\*\/]\-[1-9]\d*\.?\d*))*$/";
	if( preg_match($pattern, $exp, $matches) == 1 && $matches[0] == $exp ) {
		if ( assert('$calcVal = eval("return " . $exp . ";");') )
			print "<h2>Result<br></h2>" . $exp . " = " . $calcVal;
		else
			print "<h2>Result<br></h2>" . $exp . " = Undefined";
	}
	else
		print "<h2>Result<br></h2>" . "Invalid input expression " . $exp . ".";
}
?>

</body>

</html>