<html>
<head><title>Query</title></head>
<body>

<h1>Query</h1>

Type an SQL query in the following box:
<br>

<form action="query.php" method="get">
<textarea name="userquery" rows="8" cols="60"></textarea>
<input type="submit" name="submit" value="Submit" method>
</form>

<sub>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.</sub>
<br>
<br>

<?php
	
// Establish Connection to MySQL
$db_connection = mysql_connect("localhost", "cs143", "");

// Establish Connection to the database
mysql_select_db("CS143", $db_connection);

$query = $_GET["userquery"];
$query = get_magic_quotes_gpc() ? stripslashes($_GET['userquery']) : $_GET['userquery']; 
$result = mysql_query($query, $db_connection);
if ($result != false){
	print "<h3>Results from MySQL:";
	$numfields= mysql_num_fields($result);
		
	$x = 0;
	$y = 0;

	print "<table border = 1>";
		// Print out the bolded column types
		while($y < $numfields){
			$fieldname = mysql_field_name($result,$y);
			print "<td><b>$fieldname</b></td>";
			$y = $y + 1;
		}
		$y = 0;

		// Print out the information
		while($row = mysql_fetch_row($result)) {
			print "<tr>";
			while($x < $numfields){   
				if ($row[$x] == NULL)
					print "<td>N/A</td>";		
				else
					print "<td>$row[$x]</td>";
			    $x = $x + 1;
			}
			print "</tr>";
			$x = 0;
		}
	print "</table>";
}



mysql_close($db_connection);
?>



</body>
</html>
