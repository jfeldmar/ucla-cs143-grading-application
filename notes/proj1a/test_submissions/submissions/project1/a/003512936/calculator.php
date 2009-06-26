<!--
Name: Angel Darquea
ID: 003512936
Date: 10/03/2008
Fall 2008
-->
<html>
	<head><title>Project 1A: PHP Calculator</title></head>
	<body>
		<h1>PHP Calculator</h1>
		Type an expression in the following box (e.g., 10.5+20*3/25). 
		<p>
			<!-- Form creation -->
			<form action="./calculator.php" method="GET">
				Expression: <input type="text" name="expr">	
				<input type="submit" value="Calculate">
			</form>
		</p>

		Test cases:
		<ol>
			<li> A basic arithmetic operation:  3+4*5=23 </li>
			<li> Expr. with floating point or negative sign : -3.2+2*4-1/3 = 4.46667, 3+-2.1*2 = -1.2 </li>
			<li> Some typos inside operation (e.g. alphabetic letter): Invalid input expression 2d4+1 </li>
		</ol>
		
		<!--PHP Script-->
		<?php
			if($_GET["expr"]){
	
				//Regex to capture assigment's math expression
				//$myRegex = "/^[-+]?[0-9]*\.?[0-9]*[\-\*\+\/0-9\.]*$/";
				$myRegex = "/^[-+]?[0-9]*\.?[0-9]*[\-\*\+\/0-9\.]*$/";

				//Validate expression using regular expressions and then use it.
				if ( preg_match($myRegex, $_GET["expr"] ) == 1) {
					$myExpr = $_GET["expr"];
					if (preg_match("/[\+-\/\*]$|.*\.\d*\..*/", $myExpr) == 1)
						echo ("<h2>Result</h2>" . "1:Invalid input expression"." ".$_GET["expr"]."."."<br/>");
					else {
						eval("\$ans= $myExpr;");
						echo ("<h2>Result</h2> $myExpr = $ans <br/>");
					}
				} 
				else 
					echo ("<h2>Result</h2>" . "2:Invalid input expression"." ".$_GET["expr"]."."."<br/>");	
			}
		?>
	</body>
</html>

