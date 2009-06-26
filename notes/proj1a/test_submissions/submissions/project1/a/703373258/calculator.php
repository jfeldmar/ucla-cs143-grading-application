

<html>
	<body>

		<h1>Calculator</h1>
		Type an expression in the following box (e.g., 10.5+20*3/25).
		<form action="calculator.php" method="GET">
			<input type="text" name="Calculate" />
			<input type="submit" value="Calculate"/>
		</form>
	<li>Only numbers and +,-,* and / operators are allowed in the expression</li>
    <li>The evaluation follows the standard operator precedence.</li>
    <li>The calculator does not support parentheses.</li> 
	<?php

	if($_GET["Calculate"])
		isValid();
	function sToF($string){


		$number = 0;
		$array = str_split($string);
		$decLength = 1;
		$dec = false;
		for($itt=0;$itt<strlen($string);$itt++){
			if($array[$itt]=='.')
				$dec = true;
			else{
				$number*=decLength;
				$number*=10;
				$number+=$array[$itt]-'0';
				if($dec)
					$decLength*=10;
				$number/=$decLength;
			}
		}
		echo $number;
		return $number;
	
	}
	function isValid()
		{
		$input = $_GET["Calculate"];
		print "<h2>Result</h2>";
		$length = strlen($input);
		if($length==0)
			return true;
		echo $input." ";

		$array = str_split($input);

		for($itt = 0; $itt < $length; $itt++){
			if($array[$itt]<'0'||$array[$itt]>'9'){

				if($array[$itt]!='+' && $array[$itt]!='-' && $array[$itt]!='/' && $array[$itt]!='*' && $array[$itt]!='.'){
					echo( "Invalid input expression ".$input.'.');
					return false;
				}

				if($itt == $length-1 && $array[$itt]!='.'){
					echo( "Invalid input expression ".$input.'.');
					return false;
					
				}

				if($itt < $length-1 && ($array[$itt+1]!= '-'||$itt!=$length-1)&&($array[$itt+1]>'9'||$array[$itt+1]<'0')){
					echo( "Invalid input expression ".$input.'.');
					return false;
				}

				//if(($itt>0) && ($array[$itt-1]=='+'||$array[$itt-1]=='-'||$array[$itt-1]=='*'||$array[$itt-1]=='\')&& ($array[$itt]!='.'||$array[$itt-1]!='.'))
			}
		}	
	$numberString = "";
	$neg = "false";
	$total = 0.0;
	$sign = '';
		for($itt = 0; $itt < $length; $itt++){
			if($array[$itt]<'0'||$array[$itt]>'9')
			{
				if($sign=='+')
					$total += (float)$numberString;
				if($sign=='//')
					$total /= (float)$numberString;
				if($sign=='*')
					$total *= (float)$numberString;
				if($sign=='-'  && ($array[$itt-1]>'9'||$array[$itt-1]<'0'))
					$total -= (float)$numberString;
				$sign='';
			}
					
			if($neg==true && ($array[$itt-1]>'9'||$array[$itt-1]<'0'))
				$numberString = "-".$numberString;
			if($array[$itt]=='+')
				$sign='+';
			if($array[$itt]=='//')
				$sign='//';
			if($array[$itt]=='*')
				$sign='*';
			if($itt!=0 && $array[$itt]=='-' && ($array[$itt-1]>'9'||$array[$itt-1]<'0'))
				$sign='-';
			else if ($array[$itt]=='-')
				$neg=true;

			if(!($array[$itt]=='+'||$array[$itt]=='-'||$array[$itt]=='*'||$array[$itt]=='/'))
				$numberString = $numberString.$array[$itt];
			
		}
			if($sign=='+')
				$total += (float)$numberString;
			if($sign=='//')
				$total /= (float)$numberString;
			if($sign=='*')
				$total *= (float)$numberString;
			if($sign=='-')
				$total -= (float)$numberstring;
			echo "= ";
			echo $total;
		//$quote = preg_quote( "+-" );
		//$numbers = preg_split("/[$quote].\/.\*/",$input, -1, PREG_SPLIT_NO_EMPTY );
		//echo "\n"."numbers ".$numbers."\n";
		//$arrSize = count($numbers);
		//for($itt=0; $itt<$arrSize;$itt++){
			//echo "\n".$numbers[$itt]."\n";
			//$numbers[$itt] = sToF($numbers[$itt]);
			//}
	}
	?>
	</body>
</html>