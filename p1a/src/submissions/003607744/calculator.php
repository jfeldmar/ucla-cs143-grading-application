<html>
<head> <title> Calculator.html </title> </head>
<body>

<font size="20"><EM><B>Calculator</B></EM></font>
<br><br>

Type an expression in the following box (e.g., 1+2-3*4/5). <br><br>

<form action="calculator.php" method="get">
<input type="text" name="string" size="20">
<input type="submit" value="calculate" >
</form>
<br>
    * Only numbers and +,-,* and / operators are allowed in the expression.<br>
    * The evaluation follows the standard operator precedence.<br>
    * The calculator does not support parentheses.<br>
<br><br>

</body>
</html>

<html>
<head><title>calculator.php</title></head>
<body>
<?php
	// get the values
	$string = $_GET['string'];
	$error = "no";

	$max = strlen($string);

	// check input value
	if($max == 0)
		$error = "yes";

	if(ereg('[a-zA-Z]', $string))
		$error = "yes";
		
	$i = 0;
	$val1 = 0;
	$val2 = 0;
	$total = 0;
	$cal = "a";
	$op = "a";

	While($i<$max && $error != "yes"){
		$num = substr($string, $i, 1);	// ith value

		if(ereg('[^0-9]',$num))
			$op = $num;
			
		if($cal != "a"){
			if($cal == "*")
				$total = $val1 * $val2;
			elseif($cal == "/"){
				if($val2 == 0){
					if($num==0)
						$error = "yes";
				}else
					$total = $val1 / $val2;
			}elseif($cal == "+")
				$total = $val1 + $val2;
			elseif($cal == "-")
				$total = $val1 - $val2;	
			else
				$error = "yes";		

			if($error!="yes"){
				$val1=$total;
				$val2=0;
				$total = 0;
				$cal = "a";
			}
		}
		
		if($num >= 0 && $num <= 9 && $num !="+" && $num !="-" && $num !="*" && $num != "/"){
			
			if($val2 == 0)
				$val2 = $num;
			else
				$val2 = $val2 * 10 + $num;
						
		}elseif($num == "."){
			$val2 = $val2 * 1.00;
		}else{
			$val1 = $val2;
			$val2 = 0;
			$cal = $num;
		}
		$i++;

}
		
	if($error != "yes"){
		if($op == "a")
			$total = $string;
		else{
			if($op == "*")
				$total = $val1 * $val2;
			elseif($op == "/"){
				if($val2 == 0)
					$error = "yes";
				else
					$total = $val1 / $val2;
			}elseif($op == "+")
				$total = $val1 + $val2;
			elseif($op == "-")
				$total = $val1 - $val2;	
			else
				$error = "yes";			
		}
	}	
	$i=1;

	if($i > 0){
		echo "Result \n" ;
		
		if($error == "yes"){
			echo $string ." is not a valid entry.  Please try again! \n";
		}else{
			echo $string." = ".$total. "\n";
		}
	}
	
?>

