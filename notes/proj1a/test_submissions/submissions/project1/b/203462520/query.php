<html><head><title>CS143 Project 1B QueryBox</title></head>
<body>
<p>Type an SQL query in the following box:</p>

<form action='query.php' method="get" name="calc">
<p>
<textarea cols=60 rows=8 name='ops'>
<?php echo $_REQUEST{'ops'};?>
</textarea> <input type="submit" name='submit' value="Submit">
</p>
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
available tables.</small></p>


<?php

$ops = $_REQUEST['ops'];
if ($ops != "") {

	echo "<h3>Results from MySQL:</h3>";
	$username = "cs143";
	$password = "";
	$hostname = "localhost";
	$dbh = mysql_connect($hostname, $username, $password) or die("Unable to connect to database");

	$selected = mysql_select_db("CS143", $dbh) or die ("Couldn't connect to db CS143");

	$result = mysql_query($ops);
	
	echo "<table border='1' cellspacing='1' cellpadding='2'>\n";

	$row = mysql_fetch_array($result, MYSQL_ASSOC); # get first result and attr names

	if ($row) {

		echo "<tr>";
		foreach (array_keys($row) AS $attrname) { # print out attr names in bold
			echo "<td align='center'><b>$attrname</b></td>";
		}
		echo "</tr>";
		
		do { # print first row, and get any remaining rows
			echo "<tr>";
			foreach ($row as $attr) {
				echo "<td align='center'>";
				if ($attr != "") {
					echo "$attr";
				} else {
					echo "N/A";
				}
				echo "</td>";
			
			}
			echo "</tr>\n";
		} while ($row = mysql_fetch_array($result, MYSQL_ASSOC));

	} else {
		echo "<p>No results from this query.</p>";
	}
	mysql_close($dbh);
}

?>

</body>
</html>
