<html>
<body>

<!--Form to receive calculation input-->

<form action="calculator.php" method="get">
<input type="text" name="calculate" />
<input type="submit" value="Calculate" />
</form>


<!--PHP Block-->


<?php

$text = $_GET["calculate"];

//Check to see if the expression is valid
if(preg_match("/^(-?[0-9]+(\.[0-9]+)?)([\.+\-\/*](-?[0-9]+(\.[0-9]+)?))*$/", $text)){

//Check for division by 0
if(preg_match("/\/-?0+.?0*([\+\-\/*]|$)/",$text))
	$answer = "ERROR: Division by 0";
else 
	eval("\$answer = $text ;");

echo $text . " = " . $answer;
}


elseif($text)
	echo "Invalid input expression " . $text;

?>

</body>
</html>