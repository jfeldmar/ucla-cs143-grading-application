<header>
<title>Project 1A - Evan Lloyd</title>
</header>

<body>

<form action = "calculator.php" method="post">
<input type = "text" name="calc"/>
<br/>
<input type = "submit" value="Calculate"/>
</form>

<?php
$calc = $_POST['calc'];
if($calc == null)
  return;

$number = "-?[0-9]+\.[0-9]*|-?[0-9]+";

$result = $calc;

// Avoiding use of eval, which is generally dangerous to use from a security standpoint
// Works by reducing pairs of numbers separated by binary operators by performing the operation and then replacing the substring with the result.

// First do all multiplications
while(preg_match("/(".$number.")\*(".$number.")/", $result, $operands, PREG_OFFSET_CAPTURE))
{
  $result = substr_replace($result, $operands[1][0]*$operands[2][0], $operands[0][1], strlen($operands[0][0]));
}

// Divisions
while(preg_match("/(".$number.")\/(".$number.")/", $result, $operands, PREG_OFFSET_CAPTURE))
{
  $result = substr_replace($result, $operands[1][0]/$operands[2][0], $operands[0][1], strlen($operands[0][0]));
}

// Subtractions / additions (done together because they need to be done from left to right)
while(preg_match("/(".$number.")([+-])(".$number.")/", $result, $operands, PREG_OFFSET_CAPTURE))
{
  if($operands[2][0] == "-")
    $result = substr_replace($result, $operands[1][0]-$operands[3][0], $operands[0][1], strlen($operands[0][0]));
  else
    $result = substr_replace($result, $operands[1][0]+$operands[3][0], $operands[0][1], strlen($operands[0][0]));
}

// If input was valid, it should have been reduced to a single number.
if(!is_numeric($result))
{
  print "Syntax error in input: ".$calc;
  return;
}

print $calc . " = " . $result;
  
?>

</body>