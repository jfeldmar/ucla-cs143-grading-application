<html>
<head><title>CS143 Project 1B</title></head>
<body>
<p>Type an SQL query in the following box:</p>

<p><form action="query.php" method="GET">
	<textarea name="query" cols="60" rows="8"></textarea>
	<input type="submit" value="Submit" />
</form></p>

<p><small>Note: tables and fields are case sensitive.  Run "show tables" to 
see avaiable tables.</small></p>

<?php
//Retrieve the desired query from text box
$query = $_GET["query"];
//Open connection
$db_connection = mysql_connect("localhost", "cs143", "");
//Select the database
mysql_select_db("CS143", $db_connection);
//Query the database
$results = mysql_query($query, $db_connection);

//If no query exists, output nothing
if (!$query) {}
//If there is an error, output error message
else if (mysql_errno() != 0) {
	echo "<h3>Results from MySQL:</h3>\n";
	echo "<p>" . mysql_error() . "</p>";
}
//Otherwise
else {
	echo "<h3>Results from MySQL:</h3>\n";
	//Creating the table
	echo "<table border=1 cellspacing=1 cellpadding=2>" . "\n";
	//Title Row
	echo "<tr align=center>";
		for ($i = 0; $i < mysql_num_fields($results); $i++) {
			echo "<td><b>" . mysql_field_name($results, $i) . "</b></td>";
		}
	echo "</tr>" . "\n";
	//Row by row
	while ($row = mysql_fetch_row($results)) {
		echo "<tr align=center>";
		for ($i = 0; $i < mysql_num_fields($results); $i++) {
			echo "<td>" . $row[$i] . "</td>";
		}
		echo "</tr>" . "\n";
	}
	//Ending the table
	echo "</table>";
}
//Closing the eonnection
mysql_close($db_connection);

?>

</body>
</html>

