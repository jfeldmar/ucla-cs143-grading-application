<html>
<head>
	<title>CS143 Project 1B</title>
	<style type="text/css">
		body
		{
			font-family: Helvetica, Verdana, Geneva, Arial, sans-serif ;
			background-color: #ADD;
			
			
		}
		table.sqlresults 
		{text-align: center;
		font-family: Helvetica, Verdana, Geneva, Arial, sans-serif ;
		font-weight: normal;
		font-size: 12px;
		color: #fff;
		background-color: #666;
		border: 0px;
		border-collapse: collapse;
		border-spacing: 0px;}

		table.sqlresults td 
		{background-color: #CCC;
		color: #000;
		padding: 4px;
		text-align: center;
		border: 1px #fff solid;}

		table.sqlresults td.hed
		{background-color: #666;
		color: #fff;
		padding: 4px;
		text-align: center;
		border-bottom: 2px #fff solid;
		font-size: 12px;
		font-weight: bold;}
  </style>
	
</head>
<body>
Type an SQL query in the following box:
<p>

<?php
print "<form action=\"query.php\" method=\"GET\">
<textarea name=\"query\" cols=\"60\" rows=\"8\">" . $_GET["query"] . "</textarea>
<input type=\"submit\" value=\"Submit\" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run \"show tables\" to see the list of
available tables.</small></p>";


// Open a connection to the database
$db_connection = mysql_connect("localhost", "cs143", "");

// Use the TEST database FIXME
mysql_select_db("CS143", $db_connection);

$query = $_GET["query"];

if ($query != "")
{
	$sanitized_name = mysql_real_escape_string($name, $db_connection);
	$query_to_issue = sprintf($query, $sanitized_name);
	$rs = mysql_query($query_to_issue, $db_connection);

	$numCols = 0;
	$colNames = array ();
	// Store the column names in $colNames and save the number of columns in the table in $numCols
	while ($tableInfo = mysql_fetch_field($rs))
	{
		$colNames[$numCols] = $tableInfo->name;
		$numCols++;
	}


	print "<table class=\"sqlresults\" cellspacing=\"0\"><tr>";

	// Create the table headers
	for($i=0; $i<$numCols; $i++)
		print "<td class=\"hed\">" . $colNames[$i] . "</td>";

	print "</tr>";


	while($row = mysql_fetch_row($rs)) {
		print "<tr>";
		for($i=0; $i<$numCols; $i++)
		{
			if ($row[$i] == NULL)
				print "<td>N/A</td>";
			else
				print "<td>" . $row[$i] . "</td>";
		}
		print "</tr>";
	}

	print "</table>";
}
mysql_close($db_connection);
?>
</body>
</html>