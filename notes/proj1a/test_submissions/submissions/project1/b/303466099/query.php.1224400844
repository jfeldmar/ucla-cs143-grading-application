<html>

<head>
<title>[CS143] Internet Movie Database</title>
</head>

<body>
<p>Enter an SQL Query:</p>

<form method="POST" action="query.php">
<?php $query = $_POST["query"]; ?>
<textarea name="query" cols="60" rows="8"><?php print $query ?></textarea>
<input type="submit" value="Submit"/>
</form>

<?php
$db_connection = mysql_connect("localhost", "cs143", "");

mysql_select_db("CS143", $db_connection);

$regex_select = "/^\s*SELECT/";
$regex_show = "/^\s*SHOW/";

if(preg_match($regex_select, $query) || preg_match($regex_show, $query))
{
	print "<h3>Results from MySQL:</h3>";
	$rs = mysql_query($query, $db_connection);

	$fields = 0;

	print "<table border=1 cellspacing=1 cellpadding=2>";
	print "<tr align=center>";
	while($fields < mysql_num_fields($rs))
	{
		print "<td><b>";
		print mysql_fetch_field($rs, $fields)->name;
		print "</b></td>";
		$fields++;
	}
	print "</tr>";

	while($row = mysql_fetch_row($rs))
	{
		$fields = 0;
		print "<tr align=center>";
		while($fields < mysql_num_fields($rs))
		{
			print "<td>";
			if($row[$fields] == NULL)
				print "N/A";
			else
				print $row[$fields];
			print "</td>";
			$fields++;
		}
		print "</tr>";
	}

	print "</table>";
}
else
{
	if($query != NULL)
		print "Sorry only SHOW and SELECT queries are allowed!";
}

mysql_close($db_connection);
?>

</body>

</html>
