<html>
<head><title>Project 1b - Evan Lloyd</title></head>

<body>

<form action="./query.php" method="POST">
<p>Enter an SQL query (SELECT or SHOW) below.</p>
<p>
<textarea name="query" cols="60" rows="8"><?php echo $_POST['query'];?></textarea><br/>
<input type="submit" value="Submit" />
</p>
</form>

<?php
	$query = $_POST['query'];

	if(!$query)
		return;
	
	$select = "SELECT ";
	$show = "SHOW ";
	
	if(strncasecmp($query, $select, strlen($select)) && strncasecmp($query, $show, strlen($show)))
	{
		echo "<p>Only SELECT and SHOW queries are permitted.</p>";
		return;
	}
	
	$conn = mysql_connect("localhost", "cs143", "");
	mysql_select_db("CS143", $conn);
	
	$result = mysql_query($query, $conn);
	
	if(!$result)
	{
		echo "<p>No results returned.</p>";
		return;
	}
	
	echo "<p>Results of query:</p> <table border=1 cellspacing=4 cellpadding=4><tr>";
	
	$cols = mysql_num_fields($result);
	for($i = 0; $i < $cols; $i++)
	{
		echo "<td><b>".mysql_fetch_field($result, $i)->name."</b></td>";
	}
	
	echo "</tr>";
	
	while($row = mysql_fetch_row($result))
	{
		echo "<tr>";
		foreach ($row as $value)
		{
			echo "<td>";
			if($value)
				echo $value;
			else
				echo "(NULL)";
			echo "</td>";
		}
		echo "</tr>";
	}
	
	echo "</table></p>";
	
	mysql_close($conn);

?>

</body>
</html>