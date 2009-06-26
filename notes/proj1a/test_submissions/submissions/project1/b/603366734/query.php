<html>
<head>
<title>Edward Chang's Mini Movie Database</title>
</head>
<body>
<h1>Mini Movie Database</h1>
<font face=verdana size=2>Please type an SQL query in the box below:<br>

<form action="query.php" method="post">
<textarea name="tarea" ROWS=10 COLS=75>
</textarea><br>
<input type="submit" value="Submit">
</form><p>
<font face=verdana size=1>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.
<font size=2><p>
<?php
	$query = $_POST["tarea"];
	if ($query != "")
	{
		echo "<table border=1 cellpadding=4>";
		$db_connection = mysql_connect("localhost", "cs143", "");
		mysql_select_db("CS143", $db_connection);
		$rs = mysql_query($query, $db_connection);
		while($row = mysql_fetch_row($rs))
		{
			echo "<tr>";
			foreach ($row as $value) 
			{
				echo "<td>$value</td>";
			}
			echo "</tr>";
		}
		unset($value);
		mysql_close($db_connection);
		echo "</table>";
	}
?>

</body>
</html>
