<html>
<head><title>SQL Query</title></head>
<body>
Enter your SQL query:
<p>
<form action="query.php" method="GET">
<textarea name="query" cols="50" rows="10"></textarea>
<input type="submit" value="Submit" />
</form>
</p>

<?php
$query = $_GET["query"];
if (isset($query))
{
	echo "Your query: ".$query."<br />";
	$db_connection = mysql_connect("localhost", "cs143", "");
	if (!db_connection)
	{
		echo 'Could not connect to database.';
		exit;
	}
	if (!mysql_select_db("CS143", $db_connection))
	{
		echo 'Could not select database.';
		exit;
	}
	$rs = mysql_query($query, $db_connection);
	if (!$rs)
	{
		echo 'Could not run query: ' . mysql_error();
		exit;
	}
	$numfields = mysql_num_fields($rs);
	echo "<h1>Result:</h1>";
	echo "<table border='1'><tr>";
	for($i=0; $i<$numfields; $i++)
	{
		$field = mysql_fetch_field($rs);
		echo "<td>{$field->name}</td>";
	}
	echo "<tr>\n";
	while($row = mysql_fetch_row($rs))
	{
		echo "<tr>";
		foreach($row as $box)
			echo "<td>$box</td>";
		echo "<tr>";
	}

	mysql_close($db_connection);
}
?>

</body>
</html>
