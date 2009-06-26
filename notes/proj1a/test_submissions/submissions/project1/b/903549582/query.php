<html>
<head><title>CS143 Project 1B</title></head>
<body>
Type an SQL query in the following box:
<p>
<form action="./query.php" method="GET">
<textarea name="query" cols="60" rows="8"></textarea>
<input type="submit" value="Submit" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
available tables.</small></p>
<?php
$db_connection = mysql_connect("localhost", "cs143", "");
if(!$db_connection) {
    $errmsg = mysql_error($db_connection);
    print "Connection failed: $errmsg <br />";
    exit(1);
}

mysql_select_db("CS143", $db_connection);

//$query = "select * from Actor order by id";
$sanitized_name = mysql_real_escape_string($name, $db_connection);
$query_to_issue = sprintf($_GET["query"], $sanitized_name);

print "<h3>Input query: </h3>".$_GET["query"]."\n\n";

$rs = mysql_query($query_to_issue, $db_connection);

if(!$rs) {
	$errmsg = mysql_error($db_connection);
	print "$errmsg <br />";
    exit(1);
}

print "<h3>Results from MySQL:</h3>\n\n";
print "<table border=1 cellspacing=1 cellpadding=2>\n\n";

print "<tr align=center>";
for($i = 0; $i < mysql_num_fields($rs); $i++)
	print "<td><b>".mysql_field_name($rs, $i)."</b></td>";
print "</tr>\n";

while($row = mysql_fetch_row($rs)) {
	print "<tr align=center>";
	for($i = 0; $i < mysql_num_fields($rs); $i++)
		if($row[$i] == "")
			print "<td>N/A</td>";
		else
			print "<td>".$row[$i]."</td>";
	print "</tr>\n";
}

print "\n</table>";


?>
</body>
</html>
