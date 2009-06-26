<html>

<head>
  <title></title>
</head>

<body>

<h1>Calculator</h1>

<form action="calculator.php" method="GET">
<input type="text" name="in_equ" />
<input type="submit" />
</form>

<?php

$equ = $_GET["in_equ"];
$pattern = '/^-?(?=\d|\.\d)\d*(\.\d*)?((?=([-+*]|\/0*(\.0*)?[1-9]))[-+*\/]-?(?=\d|\.\d)\d*(\.\d*)?)*$/';

if(preg_match($pattern, $equ, $matches)){
	$zero_pattern[0] = '/^0+([1-9])/';
        $zero_pattern[1] = '/([^\d\.])0+([1-9])/';
        $zero_replace[0] = '$1';
        $zero_replace[1] = '$1$2';
	$fixed_equ = preg_replace('/--/', '+', $equ);
        $fixed_equ = preg_replace($zero_pattern, $zero_replace, $fixed_equ);
	eval("\$ans = $fixed_equ;");
	echo "$equ = $ans<br />";
        //for($i=0;$i<5;$i++){
        //	echo "matches[$i] = $matches[$i]<br />";
        //}
}
elseif(!$equ){
	echo "Please enter an equation<br />";
}
else{
	echo "Invalid input: $equ<br />";
}

?>

</body>

</html>
