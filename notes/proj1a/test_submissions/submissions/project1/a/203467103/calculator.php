<h1>Calculator</h1>
Type an expression in the following box (e.g., 10.5+20*3/25). 
<p>
</p><form method="get">
<input name="input" type="text"/><input value="Calculate" type="submit" />
</form>
<p></p>
<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression.
</li><li>The evaluation follows the standard operator precedence.
</li><li>The calculator does not support parentheses. 

</li><li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
</li></ul>

<?php

#error_reporting(E_ALL & ~(E_WARNING | E_NOTICE | E_PARSE));
error_reporting(0);
//echo var_dump($_GET);
//echo var_dump($_POST);

if (strval($_GET["input"]) == "")
  return;

$input = $_GET["input"];
  
echo "<h2>Result</h2>";

//expression = begin line, optional -, one or more 0-9, optional period,  0 or more 0-9, ([+-*/], optional -, one or more 0-9, optional period, 0 or more 0-9) 0 or more, end line
if (preg_match("/^[+\-]?[0-9]+\.?[0-9]*([+\-*\/][+\-]?[0-9]+\.?[0-9]*)*$/", $input))
{
  eval("\$solution = $input;");
  #if (eval("\$solution = $input;"."return true;"))
  #if (strval($solution) != "")
    echo $input." = ".$solution;
  #else
  #  echo $input." = Undefined";
}
else
  echo "Invalid input expression $input".".";

?>

</body>
</html>