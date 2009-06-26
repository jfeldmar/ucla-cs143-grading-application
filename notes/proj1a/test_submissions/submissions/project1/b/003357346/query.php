<html>
<head><title>CS143 Project 1B</title></head>

<body>
Type an SQL query in the following box:
<p>
<form method="GET">
<textarea name="query" cols="60" rows="8"></textarea>
<input type="submit" value="Submit" />
</form>
</p>

<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables. </small></p>

<?php

//set up connection
$db_connection = mysql_connect("localhost", "cs143", "");
mysql_select_db("CS143", $db_connection);

//test query
$query = $_GET["query"]; 
if($query == "") {

}
else {
	if(!$rs = mysql_query($query, $db_connection)) {
		print(mysql_error());
		exit(1);
	}
	$numFields = mysql_num_fields($rs);

	print "<h3> Results from MySQL query </h3>";
	print "<table border='1'>";
	print "<tr>";
	for($i=0; $i<$numFields; $i++) {
		$field = mysql_field_name($rs, $i);
		print "<td><b>$field</b></td>";
	}
	print "</tr>";
	
	while($row = mysql_fetch_row($rs)){

		print "<tr>";
		for($i=0; $i<$numFields; $i++)  {
			if($row[$i] == NULL)
				$row[$i] = 'N\A';
			print "<td>$row[$i]</td>";
		}
		print "</tr>";
	}
	print "</table>";
//	else {
	
//		print(mysql_error());
//	}

}

//close the connection
mysql_close($db_connection);
?>

</body>
</html>

