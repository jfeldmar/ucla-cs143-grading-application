<html>
	<head>
		<title>John Gang's Calculator</title>
	</head>
	
	<body>
	
		<h1>John Gang's Calculator</h1>
		Type an expression in the following box (e.g., 10.5+20*3/25). 
		<p>
			<form action="." method="GET">
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
		
			function convertStructure($data)
			{
				$temp;
				$i = 0;
				$j = 0;
				$k = 0;
				
				while($data[$j] != null)
				{
					if (is_numeric($data[$j][0]) || ($data[$j][0] == '-' && is_numeric($data[$j][1])))
					{
						$count = 0;
						
						while($data[$j][$count] != null)
						{
							if ($data[$j][$count] == '.')
							{
								break;
							}
							
							$count++;
						}
						
						$inc = 0;
						$tArray = null;
						
						while ($inc < $count)
						{
							$tArray[$inc] = $data[$j][$count - $inc - 1];
							
							$inc++;
						}
						
						while ($data[$j][$count] != null)
						{
							$tArray[$count] = $data[$j][$count];
							
							$count++;
						}
						
						$mValue = 1;
						$count = 0;
						$result = 0;
						$negFlag = 1;
						
						while ($tArray[$count] != null)
						{
							if ($tArray[$count] == '.')
							{
								$mValue = 0.1;
								$count++;
							}
							
							if ($tArray[$count] == '-')
							{
								$negFlag = -1;
							}
							else if ($tArray[$count] == 0)
							{
								$mValue = $mValue * 10;
							}
							else if ($mValue >= 1)
							{
								$result = $result + ($mValue * $tArray[$count]);
								$mValue = $mValue * 10;
							}
							else
							{
								$result = $result + ($mValue * $tArray[$count]);
								$mValue = $mValue / 10;
							}
							
							$count++;
						}
						
						if ($negFlag == -1)
						{
							$result = -$result;
							$negFlag = 0;
						}
						
						$temp[$j] = $result;
						$j++;
						
					}
					else
					{
						$temp[$j] = $data[$j][0];
						$j++;
					}
					
					$k = 0;
				}
				
				return $temp;
			}
		
			function readData($tempData)
			{
				$i = 0;
				$j = 0;
				$k = 0;
				
				$result;
				$dotFlag = 0;
				
				while ($tempData[$i] != null)
				{
					if (is_numeric($tempData[$i-1]) && $tempData[$i] == '.')
					{
						$dotFlag++;
						
						if (is_numeric($tempData[$i+1]))
						{
							$result[$j][$k] = $tempData[$i];
							$k++;
						}
						else
						{
							$result[$j][$k] = $tempData[$i];
							$k = 0;
							$j++;
							$dotFlag = 0;
						}
					}
					else if(is_numeric($tempData[$i]) && !is_numeric($tempData[$i+1]) && $tempData[$i+1] != '.')
					{
						$result[$j][$k] = $tempData[$i];
						$k = 0;
						$j++;
						$dotFlag = 0;
					}
					else if ($tempData[$i] == '-' && !is_numeric($tempData[$i-1]) && is_numeric($tempData[$i+1]))
					{
						$result[$j][$k] = $tempData[$i];
						
						$k++;
					}
					else if (($tempData[$i] == '+' || 
								$tempData[$i] == '-' || 
								$tempData[$i] == '*' || 
								$tempData[$i] == '/') && 
							(is_numeric($tempData[$i+1]) || ($tempData[$i+1] == '-' && (is_numeric($tempData[$i+2]) || $tempData[$i+2] == '.'))))
					{
						$k = 0;
						$result[$j][$k] = $tempData[$i];
						$j++;
						$dotFlag = 0;
					}
					else if (is_numeric($tempData[$i]))
					{
						$result[$j][$k] = $tempData[$i];
						
						$k++;
					}
					else
					{
						return "error";
					}
					
					if ($dotFlag > 1)
					{
						return "error";
					}
					
					$i++;
				}
				
				return $result;
			}
			
			function calculate($data)
			{
				$i = 0;
				
				while ($data[$i] != null)
				{
					$flag = 0;
					while ($data[$i] != null)
					{
						if ($data[$i] == '*')
						{
							$temp = $data[$i - 1] * $data[$i + 1];
							$flag++;
							
							$j = 0;
							$k = 0;
							$tempData = null;
							while($data[$k] != null)
							{
								if ($j == ($i - 1))
								{
									$tempData[$j] = $temp;
									$k = $k + 2;
								}
								else
									$tempData[$j] = $data[$k];
									
								$j++;
								$k++;
							}
							$data = $tempData;
							$i = 0;
						}
						else if ($data[$i] == '/' && $data[$i+1] == 0)
						{
							return "undfined";
						}
						else if ($data[$i] == '/')
						{
							$temp = $data[$i - 1] / $data[$i + 1];
							$flag++;
							
							$j = 0;
							$k = 0;
							$tempData = null;
							while($data[$k] != null)
							{
								if ($j == ($i - 1))
								{
									$tempData[$j] = $temp;
									$k = $k + 2;
								}
								else
									$tempData[$j] = $data[$k];
									
								$j++;
								$k++;
							}
							$data = $tempData;
							$i = 0;
						}
						$i++;
					}
				}
				
				$i = 0;
				
				while ($data[$i] != null)
				{
					$flag = 0;
					while ($data[$i] != null)
					{
						if ($data[$i] == '+')
						{
							$temp = $data[$i - 1] + $data[$i + 1];
							$flag++;
							
							$j = 0;
							$k = 0;
							$tempData = null;
							while($data[$k] != null)
							{
								if ($j == ($i - 1))
								{
									$tempData[$j] = $temp;
									$k = $k + 2;
								}
								else
									$tempData[$j] = $data[$k];
									
								$j++;
								$k++;
							}
							$data = $tempData;
							$i = 0;
						}
						else if ($data[$i] == '-')
						{
							$temp = $data[$i - 1] - $data[$i + 1];
							$flag++;
							
							$j = 0;
							$k = 0;
							$tempData = null;
							while($data[$k] != null)
							{
								if ($j == ($i - 1))
								{
									$tempData[$j] = $temp;
									$k = $k + 2;
								}
								else
									$tempData[$j] = $data[$k];
									
								$j++;
								$k++;
							}
							$data = $tempData;
							$i = 0;
						}
						$i++;
					}
				}
				
				return $data[0];
			}
		
			$read = $_REQUEST["expr"];
			$request = readData($read);
			
			if ($request == "error")
			{
				echo "<h2>Your input is <b>INVALID</b> - (".$read.")<h2>";
			}
			else if($request != null)
			{
				$result = calculate(convertStructure($request));
				
				if ($result != null)
				{
					echo "<h1>Result<h1>";
					echo "<h2>".$read." = ".$result."</h2>";
				}
			}
		?>
	
	</body>
</html>
