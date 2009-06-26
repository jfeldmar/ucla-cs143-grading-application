<html>
<head>
<style type="text/css">
table#csstable {
	background-color: #fafafa;
	border: 1px black solid;
	border-spacing: 2px;
}
table#csstable td {
	text-align: center;
	background: beige;
}
table#csstable td:hover {
	background: yellow;
}
</style>
<title>Web Query</title>
</head>

<body>
<h1>Web Query Interface</h1>

Type an SQL query in the following box:
<p>
<form method="GET">
<textarea name="query" cols="60" rows="8"></textarea>
<input type="submit" value="Submit" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
available tables.</small></p>

<?php

	if (strval($_GET["query"]) == "")
		return;

	$db_connection = mysql_connect("localhost", "cs143", "");
	
	if(!$db_connection) {
	    $errmsg = mysql_error($db_connection);
	    print "Connection failed: $errmsg <br />";
	    exit(1);
	}

	
	mysql_select_db("CS143", $db_connection);

	$query = $_GET["query"];
	
	#echo $query."<br />";
	
	$sanitized_name = mysql_real_escape_string($name, $db_connection);
	$query_to_issue = sprintf($query, $sanitized_name);

	$rs = mysql_query($query_to_issue, $db_connection);
	
	$rs = mysql_query($query, $db_connection);
	
	echo "<h2>Results from MySQL</h2>";
	
	if (!$rs) {
		$errmsg = mysql_error($db_connection);
		print $errmsg;
		exit(1);
	}
	
	echo "<table id=\"csstable\"  border=\"2\">";
	echo "<tr>";
	for($i = 0; $i < mysql_num_fields($rs); $i++) {
		$field = mysql_fetch_field($rs, $i);
		echo "<td>$field->name</td>";
	}
	echo "</tr>";
	
	while($row = mysql_fetch_row($rs)) {
		echo "<tr>";
		for($i = 0; $i < mysql_num_fields($rs); $i++) {
			if ($row[$i])
				echo "<td>$row[$i]</td>";
			else
				echo "<td>N/A</td>";
			
		}
		echo "</tr>";
	}
	
	echo "</table>";
	
	
	mysql_close($db_connection);
?>

</body>
</html>