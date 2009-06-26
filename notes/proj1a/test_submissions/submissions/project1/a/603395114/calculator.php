<html>
<head><title>Calculator</title></head>
<body>

<h1>Calculator</h1>
Type an expression in the following box (e.g., 10.5+20*3/25). 
<p>
<form action="./calculator.php" method="GET">
<input type="text" name="expr"><input type="submit" value="Calculate">
</form>
</p>
<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression.
<li>The evaluation follows the standard operator precedence.
<li>The calculator does not support parentheses. 
</ul>
<?php
	if(isset($_GET['expr'])){
		echo "<h2>Result</h2>";
		$number = '(-?\d*(\.\d+)?)';
		$operators = '[\+\-\*\/\%]';
		$regex = '/^('.$number."((".$operators.$number.")+)?)$/";
		//$r = preg_match($regex,$_GET['expr'],$matches,PREG_OFFSET_CAPTURE);
		//echo "Output ".$r." w/ "."<br/>";
		//print_r($matches);
		if(preg_match($regex,$_GET['expr'],$matches,PREG_OFFSET_CAPTURE))
		{
			echo  $_GET['expr']."=".eval("return ".$_GET['expr'].";");
		}
		else{
			echo "Invalid input expression ".$_GET['expr'].".";
		}
	}
?>

</body>
</html>
