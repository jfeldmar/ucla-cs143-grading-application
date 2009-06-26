<html>
<body>

Type an SQL query in the folowing box: <br /> <br />

<form action="query.php" method="GET"> 
<input type="text" name="equ" SIZE=100 MAXLENGTH=1000 />
<input type="submit" /> 

</form>
<br />
Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.

<?php

if(isset($_GET["equ"]))
{
	$equ= $_GET["equ"];
                
	echo "RESULT <br /> <br />";

	$x = mysql_connect ("localhost", "cs143", "");

	if (!$x)
	{
		die('Could not connect: ' . mysql_error());
	}
	$db_selected = mysql_select_db("CS143",$x);

	$result = mysql_query($equ, $x);
	if (!$result)
	{
		echo "Sorry only SELECT and SHOW queries are allowed";
	}   
	else
	{
		$var = 0;
		$fieldnum = mysql_num_fields($result);
	
		while($var < $fieldnum)
		{
			echo mysql_field_name($result, $var);
			echo "	|	";
			$var += 1;
		}

		echo "<br />";

		while ($fetch = mysql_fetch_row($result)) 
		{ 
			foreach ($fetch as $row) 
			{ 
				echo $row;
				echo "	|	"; 
			} 
			echo "<br />";

		} 
	}	

}

?>
</body> 
</html> 

 

 
 
 



