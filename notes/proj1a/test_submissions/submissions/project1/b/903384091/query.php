<html>
<head><title>CS143 Project 1B</title></head>
<style type="text/css">
<!--
body, table
{
	font-family: arial;
	font-size: 12pt;
}
textarea
{
	font-family: arial;
	font-size: 10pt;
}
td
{
	text-align: center;
}
td.name
{
	font-weight: bold;
}
td.gray
{
	background-color: #E8E8E8;
}
-->
</style>
<body>
Type an SQL query in the following box:
<p>
<form action="query.php" method="GET">
<textarea name="query" cols="60" rows="8" style="border:1px solid #000"><?php $query = $_GET["query"]; print $query ?></textarea>
<input type="submit" value="Submit" style="border:0;background-color:#000000;color:#ffffff" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.</small></p>

<?php
# Open connection to database
$db_connection = mysql_connect("localhost", "cs143", "");

# Exit if connection cannot be established
if(!$db_connection) {
    $errmsg = mysql_error($db_connection);
    print "Connection failed: $errmsg <br />";
    exit(1);
}

# Select database
mysql_select_db("CS143", $db_connection);

# Ignore empty queries
if (!empty($query))
{
	print "<h3>Results from MySQL:</h3>";
	
	# Sanitize the input before sending it to MySQL
	$sanitized_name = mysql_real_escape_string($name, $db_connection);
	$query_to_issue = sprintf($query, $sanitized_name);
	
	# Send the query to MySQL
	$rs = mysql_query($query_to_issue, $db_connection);

	# Get number of columns
	$cols = mysql_num_fields($rs);
	
	#print "The number of columns is: $cols.<br />";
	
	# Print out a table
	print "<table border=\"1\" cellspacing=\"1\" cellpadding=\"1\"><tr>";
	
	# Print out each field's name
	for ($i = 0; $i < $cols; $i++)
	{
		$name = mysql_field_name($rs, $i);
		print "<td class=\"name\"> $name </td>";
	}
	print "</tr>";
	
	$switch = "gray";
	
	# Retrieve results
	while($row = mysql_fetch_row($rs)) {
		print "<tr>";
		for ($i = 0; $i < $cols; $i++)
		{
			$fld = $row[$i];
	    print "<td class=\"$switch\">$fld</td>";
		}
		print "</tr>";
		if ($switch == "gray")
		{
			$switch = "";
		}
		else
		{
			$switch = "gray";
		}
	}
	
	print "</table>";

	# Print number of affected rows
	#$affected = mysql_affected_rows($db_connection);
	#print "Total affected rows: $affected<br />";
}

# Close database
mysql_close($db_connection);
?>

</body>
</html>