<!-- Tanya Gillis CS 143 Project 1A -->

<html>

<title> Calculator </title>

<head>
<h1>
Calculator </br>
</h1>
</head>

<?php
echo "Type an expression in the following box (e.g., 10.5+20*3/25).";
?>

<form action="calculator.php" method="get">
<input type="text" name="formula"/>
<input type="submit" value="Calculate"/>
</form>

<?php

$equ = $_GET["formula"];

if(preg_match("/[a-z]/",$equ))
{
  echo "Invalid input expression.";
  exit;
}


if(preg_match("/[A-Z]/",$equ))
{
  echo "Invalid input expression.";
  exit;
}

eval("\$ans = $equ;");
echo "$equ = " .$ans;

?>

</body>
</html>
