<?php
$myline = $_POST["myline"];
if($myline) {
$space_found = preg_match('/ /', $myline, $matches);
$divide_by_zero = preg_match('/\/0/', $myline, $matches);
$output;
if ($space_found || $divide_by_zero) {
    $output = "Invalid Expression!";
} else {
    eval("\$output = $myline;");
}
echo "<h1>Result:</h1>";
echo "$myline = $output";
}
?>

<html>
<head><title>Calculator</title></head>
<body>

<h1>Calculator</h1>
Type an expression in the following box (e.g., 10.5+20*3/25). 
<p>
<form method="POST" ACTION="<?php echo $PHP_SELF;?>">
<input type="text" name="myline" size=20>
<input type="submit" name="submit" value="Calculate">
</form>
</p>
<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression.
<li>The evaluation follows the standard operator precedence.
<li>The calculator does not support parentheses. 

<li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
</ul>

</body>
</html>


<HTML>
<TITLE>
Computer Science 143
</TITLE>
<body>
</HTML>

