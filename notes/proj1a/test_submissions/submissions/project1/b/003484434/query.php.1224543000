<html>
<head><title>CS143 Project 1B</title></head>
<body>
Type an SQL query in the following box:
<p>
<form action="query.php" method="GET">
<textarea name="query" cols="60" rows="8"></textarea>
<input type="submit" value="Submit" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
available tables.</small></p>

<?php
eval("\$query = \$_GET[\"query\"];");

if($query != "")
{
	echo "<h3>Results from MySQL</h3>";
	$link = mysql_connect("localhost", "cs143", "");
	mysql_select_db("CS143", $link);

	$rs = mysql_query($query, $link);
	$num_cols = 0;
	
	echo "<table border=1 cellspacing=1 cellpadding=2>";
	echo "<tr align=center>";

	while($fields = mysql_fetch_field($rs))
	{
		$num_cols++;
		echo "<td><b>" . $fields->name . "</b></td>";
	}

	echo "</tr>";

	while($row = mysql_fetch_row($rs)) {
		echo "<tr align=center>";
		for($i = 0; $i < $num_cols; $i++)
		{	
			if($row[$i] == null)
				echo "<td>N/A</td>";
			else
				echo "<td>" . $row[$i] . "</td>";
		}
		echo "</tr>";
	}

	echo "</table>";
	mysql_close($link);
}
?>
</body>
</html>
