<html>
<body>

<form action="query.php" method="get">
<textarea NAME="input" ROWS=20 COLS=60>
</textarea>
<input type="submit" value="Query" />
</form>

<?php
$db_connection = mysql_connect("localhost", "cs143", "");
mysql_select_db("CS143", $db_connection);
$query = $_GET["input"];
$result = mysql_query($query, $db_connection);

if($result){
$fields = mysql_num_fields($result);
for ($counter = 0; $counter < $fields; $counter++)
echo "| " . mysql_field_name($result, $counter) . " |";
print "<br />";

while($row = mysql_fetch_row($result)) {
for ($counter = 0; $counter < $fields; $counter++) {

if ($row[$counter] === NULL)
print "| N/A |";

else
print "| $row[$counter] |";
}

print "<br />";
}

}

mysql_close($db_connection);
?>

</body>
</html>