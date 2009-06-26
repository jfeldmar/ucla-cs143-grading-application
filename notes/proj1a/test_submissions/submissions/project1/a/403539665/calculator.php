<html>

<head> <b> <font size = 6 type=Arial> Calculator </font> </b> </head>

<br>
<br>

<form action="calculator.php" method="GET">


Type and expression in the following box (e.g., 10.5+20*3/25).
<br>
<br>
<input type = TEXT name=userfield>
<input type = submit name=submit value = 'Calculate'>

</FORM>

<?php
error_reporting(0);

$formula = $_GET['userfield'];

eval("\$result = $formula;");


?>


<ul>
<li>Only numbers and +,-,*, and / operators are allowed in the expression.
<li>The evaluation follows the standard operator precedence.
<li>The calculator does not support parentheses
<li>The calculator handles invalid input "gracefully". It does not ouput PHP error messages.

<br><br><br>

<?php

error_reporting(0);

if($_GET['submit'] && $formula != null)
{

	if(preg_match("/[a-zA-Z]/",$formula) == 1)
	//if($matches[0] || $matches[1])
	{
		echo "Invalid input expression ".$formula;
	}
	else
	{

?>

		<b><font size = 4> RESULT</font></b>

		<br><br>

<?php

		echo "$formula = ".$result;
	}
}

?>


</html>