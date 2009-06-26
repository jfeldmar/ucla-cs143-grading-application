<html>
<head><title>Calculator</title></head>
<body>

<h1>Calculator</h1>
Type an expression in the following box (e.g., 10.5+20*3/25). 
<p>
<form action="calculator.php" method="GET">
<input type="text" name="expr"><input type="submit" value="Calculate">
</form>
</p>
<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression.
<li>The evaluation follows the standard operator precedence.
<li>The calculator does not support parentheses. 
<li>The calculator handles invalid input "gracefully". It does not output PHP error messages.
</ul>

<?php
function is_valid($n)
{
  switch ($n)
  {
    case '+':
    case '-':
    case '/':
    case '*':
      return true;
  }
  return false;
}

function getAnswer($expr)
{
  $max = strlen($expr);
  
  # Cannot divide by zero
  $match = array();
  $offset = 0;
  while ($offset < $max && preg_match("/\/0/",$expr, $match, PREG_OFFSET_CAPTURE, $offset) == 1)
  {
    # If "/0" is found, find a number between 1 and 9 after it
    $offset = $match[0][1] + 1;
	if ($offset < $max)
	{
	  $nmatch = array();
      if (preg_match("/[1-9]/", $expr, $nmatch, PREG_OFFSET_CAPTURE, $offset) == 1)
	  {
	    for ($i = $offset; $i < $nmatch[0][1]; $i++)
	    {
	      # If a number 1-9 is found, and there is an operator between the /0 and it, then it is undefined
	      if (is_valid($expr[$i]))
		    return $expr . " = Undefined";
	    }
	  }
	  else
	    return $expr . " = Undefined";
	}
  }
  
  # Last character must be a number or '.'
  if (!is_numeric($expr[$max-1]) && $expr[$max-1] != '.')
    return "Invalid input expression " . $expr . ".";
  
  $i = 0;  
  
  # Stop at end of input
  while($i < $max)
  {
     # There cannot be two symbols in a row
	 if ((is_valid($expr[$i]) || $expr[$i] == '.') && $i != 0)
	   if (is_valid($expr[$i-1]) || $expr[$i-1] == '.')
	     return "Invalid input expression " . $expr . ".";
     # Only numbers, operators, and '.' allowed
     if (!is_numeric($expr[$i]) && !is_valid($expr[$i]) && $expr[$i] != '.')
       return "Invalid input expression " . $expr . ".";
     else
       $i++;
  }
  
  # Evaluate the expression
  eval("\$result = " . $expr . ";");
  
  return $expr . " = " . $result;
}

$expr = $_GET["expr"];

# Ignore empty fields
if (!empty($expr))
  print "<h2>Result</h2>" . getAnswer($expr);	
?>
</body>
</html>