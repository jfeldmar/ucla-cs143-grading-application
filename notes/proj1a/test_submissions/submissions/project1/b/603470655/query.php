<html>

<head>
<?php $title = "SQL Web Query Interface" ?>
<title><?php print "$title"; ?></title>
</head>

<body bgcolor=white>
<h1><?php print "$title"; ?></h1>

<?php print "Type an SQL query in the following box:" ?>
<br /> <br />

<form method="POST" action="./query.php">
<TEXTAREA NAME="query" ROWS=10 COLS=60></TEXTAREA>
<input type="submit" value="Submit"/>
</form>

<h5><?php print "Note: tables and fields are case sensitive. Run \"show tables\" to see the list of available tables." ?></h5>

<?php

// Connect to MySQL:
$db_connection = mysql_connect("localhost", "cs143", "");
if(!$db_connection) {
	$errmsg = mysql_error($db_connection);
	print "Connection failed: $errmsg\n";
	exit(1);
}

// Select appropriate database:
$db_name = "CS143";
$select_db = mysql_select_db($db_name, $db_connection);
if(!$select_db) {
	$errmsg = mysql_error($db_connection);
	print "Selection of database $db_name failed: $errmsg\n";
	exit(1);
}

$query = $_POST["query"];
if($query) {
	echo "<h3>Results from MySQL:</h3>";
	$printed_colnames = false;
	
	// Perform MySQL query:
	$rs = mysql_query($query, $db_connection) or die("Query failed: " . mysql_error());
	
	// Print results:
	echo "<table border=2>";
	while ($row = mysql_fetch_array($rs, MYSQL_ASSOC)) {
		// Print column names only once:
		if (!$printed_colnames)
		{
			echo "<tr>";
			foreach (array_keys($row) AS $colname)
				echo "<th>$colname</th>";
			echo "</tr>";
			$printed_colnames = true;
		}
		
		echo "<tr>";
		foreach ($row as $col_value) {
			if($col_value) echo "<td align=\"center\">$col_value</td>";
			else echo "<td align=\"center\">N/A</td>";
		}
		echo "</tr>";
	}
	echo "</table>";
	
	// Free result set
	mysql_free_result($rs);
}

// Close MySQL connection:
mysql_close($db_connection);
?>

</body>

</html>
