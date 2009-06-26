<html>
<title>Please work</title>
<body>

Jonathan B. Lee <br>
603445374<br><br>

<form action="query.php" method="post">
SQL Query:<br> 
<TEXTAREA NAME="query" ROWS=10 COLS=60>
</TEXTAREA><br>
<input type="submit" />
</form>

<b>Result:</b><br>

<?php
$db_connection = mysql_connect("localhost", "cs143", "");	//connect to sql server
if(!$db_connection) {
    $errmsg = mysql_error($db_connection);
    print "Connection failed:" . $errmsg . "<br />";
    exit(1);
}
mysql_select_db("CS143", $db_connection);	//database selection
if ($query = $_POST["query"]) {
	$sanitized_name = mysql_real_escape_string($name, $db_connection); //input sanitization
	$query_to_issue = sprintf($query, $sanitized_name);
	$result = mysql_query($query, $db_connection);	//input query
	$numfields = mysql_num_fields($result);	//get number of columns
	echo "<table border>\n<tr>";
	for ($i=0; $i < $numfields; $i++) {
		echo '<th align="center">'.mysql_field_name($result, $i).'</th>';	//output field names
	}
	echo "</tr>\n";
	while ($row = mysql_fetch_row($result)) { 
		echo '<tr align="center"><td>'.implode($row,'</td><td>')."</td></tr>\n";	//output data
	}
	echo "</table>\n";
}
else {
	echo "Please input a query.";
}
mysql_close($db_connection);
?>

</body>
</html>