<html>

<head>
<title>Calculator</title>
</head>

<body>
<div align="center">
  <center>
<form method="GET" action="calculator.php">
<table border="0" cellspacing="1" id="calculator">
  <tr>
    <td align="center">
    <h3>Calculator</h3>
    </td>
  </tr>
<tr>
<td><p>Welcome to Sona's Calculator, I can do simple arithmetic! Please enter your expression below: <br><br>
</p>
</td>
</tr>
  <tr>
    <td align="center">
    <input type="text" name="this" size="50">
	<input type="submit" value="Calculate" name="do">
	</td>
  </tr>
  <tr />
  <tr>
    <td>
    <ul>
	<li>Only numbers and +,-,* and / operators are allowed in the expression.
    <li>The evaluation follows the standard operator precedence.
    <li>The calculator does not support parentheses.
    <li>The calculator handles invalid input "gracefully". It does not output PHP error messages. 
	</ul>
    </td>
  </tr>
<?php
if(isset($_GET['this'])){
	$expression = $_GET['this'];
	
	$myRegEx = '/^'.'(-?\d+(\.\d+)?)'.'('.'[\+\-\*\/]'.'(-?\d+(\.\d+)?)'.')*$/';
	
	if(preg_match($myRegEx,$expression))
	{
		eval("\$ans= $expression;");
		$result = $expression." = ".$ans;
	}
	else{
		$result = $expression." is invalid input. Please try again.";
	}
}
?>

  <tr>
    <td align="center"><br><strong>Result: </strong><?php echo($result); ?></td>
  </tr>
 </table>
</form>
  </center>
</div>
</body>

</html>
