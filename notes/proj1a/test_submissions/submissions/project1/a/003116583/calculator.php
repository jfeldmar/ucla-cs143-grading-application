<html>
<body>

Calculator <br /> <br />

Type an expression in the following box (e.g., 10.5+20*3/25).

<form action="calculator.php" method="GET"> 
<input type="text" name="equ" />
<input type="submit" /> 

</form>


Only numbers and +,-,* and / operators are allowed in the expression. <br />
The evaluation follows the standard operator precedence. <br />
The calculator does not support parentheses. <br />
The calculator handles invalid input "gracefully". It does not output PHP error messages. <br />
<br />


<?php


if(isset($_GET["equ"]))
{
	$equ= $_GET["equ"];
        $expression = "#^((\d+\.\d+|\.\d+|\d+\.|\d+)|(\-\d+\.\d+|\-\.\d+|\-\d+\.|\-\d+))([-+/*]((\-\d+\.\d+|\-\.\d+|\-\d+\.|\-\d+)|(\d+\.\d+|\.\d+|\d+\.|\d+)|(\+\d+\.\d+|\+\.\d+|\+\d+\.|\+\d+)))*$#";
        preg_match($expression, $equ, $matches);
        
	echo "RESULT <br /> <br />";

	if($matches[0] == $equ)
        {
                eval("\$ans= $equ;");
                echo $equ;
		echo  " = "; 
		echo $ans;

        }
        else
        {
                echo "Invalid Input Expression " .$equ;
        }
}

?>


</body> 
</html> 

 

 
 
 



