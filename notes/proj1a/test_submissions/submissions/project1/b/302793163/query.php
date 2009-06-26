<html>
<head><title>CS143 Project 1B</title></head>
<body>
Type an SQL query in the following box:

<form action="query.php" method = "post">
<textarea name = "query" cols = "60" rows = "8">
</textarea>
<input type="submit" value = "submit">
</form>

<?php
$query = $_POST["query"];
$db_connection = mysql_connect("localhost", "cs143", "");
mysql_select_db("CS143", $db_connection);
//mysql_select_db("TEST", $db_connection);
$rs = mysql_query($query, $db_connection);
if(!$rs) { exit; }

while ( $row = mysql_fetch_row($rs) ){
	$i = 0;
	while ( $row[$i] ) {
		print "$row[$i]".", ";
		$i++;
	}
	print "<br>";
}



mysql_close($db_connection);
?>


</body>
</html>