<?php print('<?xml version = "1.0" encoding = "utf-8"?>')?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns = "http://www.w3.org/1999/xhtml">
	<head>
		<title>CS 143 Calculator</title>
	</head>
	<body>
		<p><h1>CS 143 Calculator<h1></p>
		<p><h3>Type and expression in the following box(e.g., 10.5+20*3/25).<h3></p>
		<p>
			<form name = "Calculate" method = "POST" action = "calculator.php">
				<input type = "text" name = "equation">
				<input type = "Submit" name = "submit" value = "Calcualate">
			</form>
		</p>
		<?php
			if(isset($_POST["submit"]))
			{
				if(empty($_POST["equation"]))
				{
					$equstring = $_POST["equation"];
					print("\"$equstring\" is an invalid input!");
				}
				else if(preg_match( "/^([0-9]*\.?[0-9]*[\*\/\+\-][0-9]*\.?[0-9]*)+$/", $_POST["equation"]))
				{
					$fcn = $_POST["equation"];
					eval("\$answer = $fcn;");
					print("Result:  $fcn = $answer");
				}
				else if(ereg("^[0-9]*\.{0,1}[0-9]*$", $_POST["equation"]))
				{
					$fcn = $_POST["equation"];
					eval("\$answer = $fcn;");
					print("Result:  $fcn = $answer");
				}
				else
				{
					$equstring = $_POST["equation"];
					print("\"$equstring\" is an invalid input!");
				}
					
			}
		?>
	</body>
</html>