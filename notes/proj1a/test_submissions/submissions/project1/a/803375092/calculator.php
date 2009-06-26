<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>CS 143: PHP Calculator</title>
</head>

<body>
<h1>Calculator</h1>
<p>Type an expression in the following box (e.g., 10.5+20*3/25). </p>
<? 
$mult_pattern='/^(.+[^.0-9])*?(-{0,1}\d+\.{0,1}\d*\*-{0,1}\d+\.{0,1}\d*)(.*)/';
$div_pattern= '/^(.+[^.0-9])*?(-{0,1}\d+\.{0,1}\d*\/-{0,1}\d+\.{0,1}\d*)(.*)/';
$add_pattern= '/^(.+\D)*?(-{0,1}\d+\.{0,1}\d*\+-{0,1}\d+\.{0,1}\d*)(.*)/';
$tmp_result = 0;

function convertnegative($convert_string)
{
  	$subtract_pattern= '/(.*[^.0-9])?(-{0,1}\d+\.{0,1}\d*)-(-{0,1}\d+\.{0,1}\d*)(.*)/';
	while(preg_match($subtract_pattern,$convert_string,$my_matches)){
		$convert_string = $my_matches[1] . $my_matches[2] . '+-' . $my_matches[3] . $my_matches[4];
	}
	return $convert_string;
}

function calculate($opt,$function){
	$function = convertnegative($function);
	while(preg_match($opt,$function,$matches)){
		eval('$tmp_result = ' . $matches[2] . ";");
		$function = $matches[1] . $tmp_result . $matches[3] ;
		echo $function . "<br>";
	}
	return $function;
}

$string=$_POST['calculator']; 
$original_string = $string;
echo $original_string . "<br>";
$string=calculate($mult_pattern,$string);
$string=calculate($div_pattern,$string);
$string=calculate($add_pattern,$string);

?>
<form id="form1" name="form1" method="post" action="">
  <input type="text" name="calculator" id="calculator" value="<? echo $original_string; ?>"/>
  <input type="submit" name="calculate" id="calculate" value="calculate" />
</form>

<?
if ($string){
	if (preg_match('/^-{0,1}\d*\.{0,1}\d+$/',$string)){
		echo "Result: " . $string . "<br>";
	} else {
		echo "Invalid input expression: $original_string <br>";
	}
}
?>
</body>
</html>
