<html>
	<head>
		<title>Calculator
		</title>
	</head>

	<body>
		<h1>Calculator</h1>

		<p>
			Type an expression in the following box (e.g., 10.5+20*3/25).
		</p>

		<form action="calculator.php" method="GET">
			<input type="text" name="expr" />
			<input type="submit" value="calculate"/>
		</form>
		
		<p>

			<ul>
				<li>Only numbers and +,-,* and / operators are allowed in the expression.</li>
				<li>The evaluation follows the standard operator precedence.</li>
				<li>The calculator does not support parentheses.</li>
				<li>The calculator handles invalid input "gracefully". It does not output PHP error messages.</li>
			</ul>
			
		</p>
		
		<?php
			error_reporting(0);
			
			if($_GET["expr"])
			{			
				
				echo "<h2> Result </h2> <p>";			
				$number = '-?(([0-9]+(\.[0-9]*)?)|(\.[0-9]+))';
				$equ = $_GET["expr"];
				
				preg_match('/'.$number.'([-\+*\/]'.$number.')*/', $equ, $matches);

				if(preg_match('/[^0-9+*\/.\-]/', $equ, $match))
				{
					echo "Invalid characters in expression \"" .$equ."\"" .".";
				}
				else if($equ == $matches[0])
				{
					eval("\$ans = $equ ;");
					echo $_GET["expr"] ." = " .$ans;				
				}
				else			
				{
					echo "Invalid arithmetic expression \"" .$equ."\"" .".";
				}
			}
		
			echo "</p>";
		?>
		
	</body>
</html>