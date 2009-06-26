<html>
<body>
<h1>Calculator</h1>

<form action="./calculator.php" method="GET">
<input type="text" name="expr">
<input type="submit" value="Calculate">
</form>

<?php
$subject = $_GET["expr"];
$pattern = '/((\-)?\d+(.\d+)?)([*+\/\-]((\-)?\d+(.\d+)?))*/';
$divpat1 = '/\/0+$/';
$divpat2 = '/\/0+[*+\/\-]/';

$ret = preg_match($pattern, $subject, $matches);

if(strlen($subject) == 0)
	;
elseif($ret == 1 && $subject == $matches[0]) {
	$ret1 = preg_match($divpat1, $subject, $divmat1);
	$ret2 = preg_match($divpat2, $subject, $divmat2);

	if(!$ret1 && !$ret2)
	{
		eval("\$re=$subject;");
		echo $subject."=".$re;
	}
	else
		echo "Cannot be divided by 0, ".$subject.".";
}
elseif($ret == 1 && $subject != $matches[0])
	echo "Invalid input expression ".$subject.".";
else
	echo "Invalid input expression ".$subject.".";
?>

</body>
</html>
