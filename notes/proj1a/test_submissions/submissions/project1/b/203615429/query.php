<html> 
	<head><title>CS143 Project 1B</title></head> 
	<body> 
		<b>
		Chi Kin Cheang ~ 203-615-429 <br>
		beef@ucla.edu
		</b>
		<br>
		Type an SQL query in the following box:
		<p>
			<form action="<?=$_SERVER['PHP_SELF']?>" method="GET"> 
				<textarea name="query" cols="60" rows="8"><?=$_GET['query']?></textarea> 
				<input type="submit" value="Submit" /> 
			</form> 
		</p> 
		<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.</small></p> 
 
<?php
if( $_REQUEST['query'] )
{
	// Get the query
	$query = $_REQUEST['query'];
	
	echo "<h3>Results from MySQL:</h3>";
	
	// Connect to mysql server
	$connect = mysql_pconnect("localhost", "cs143") or die("Error - MySql server.");
	mysql_select_db("CS143", $connect) or die("Error - Database");
	
	// Submit the query to mysql
	$result = mysql_query($query) or die("Error - Query <br>".mysql_error()."<br>Please make sure the inputed query is correct.");
	
	// Get the # of rows of the result
	$rows = mysql_num_fields($result);
	
	// Create the output table
	echo "<table border=1 cellspacing=1 cellpadding=2>";
	
	// Create table header
	echo "<tr align=center>";
	for( $i=0; $i<$rows; $i++ )
	{
		echo "<td><b>".mysql_field_name($result, $i)."</b></td>";
	}
	echo "</tr>";
	
	// Output the data
	while( $row = mysql_fetch_array($result) )
	{
		echo "<tr align=center>";
		for( $i=0; $i<$rows; $i++ )
		{
			echo "<td>".$row[$i]."</td>";
		}
		echo "</tr>";
	}
	
	echo "</table>";
}
?>
 
</body> 
</html> 


