<html>
<head>
	<title>CS143 Project 1B Movie Database</title>
</head>
<body>
	<p>Type an SQL query in the following box:</p>
	<p>	<form action="query.php" method="GET">
		<?php
		if ($_GET["query"])
			echo '<textarea name="query" cols="60" rows="8">'.$_GET["query"].'</textarea>';
		else
			echo '<textarea name="query" cols="60" rows="8"></textarea>';
		?>
		<input type="submit" value="Submit" />
		</form>	</p>
	<p><small>Note: tables and fields are case sensitive.</small></p>
	
	<?php
	if ($_GET["query"])
	{
		$query = $_GET["query"];
		$db_name = "CS143";
		$endl = "\n";
		
		echo "<h3>Results from MySQL:</h3>".$endl;
		$mySQL = mysql_connect("localhost", "cs143", "");
		if ($mySQL)
		{
			mysql_selectdb($db_name, $mySQL);
			$resource = mysql_query($query, $mySQL);
			if ($resource)
			{
				echo "<table border=1 cellspacing=1 cellpadding=2 Style='empty-cells: show'>".$endl;
				$length = mysql_num_fields($resource);
				
				echo "<tr align=center>";
				for ($i = 0; $i < $length; $i++)
				{
					$header=mysql_field_name($resource, $i);
					echo "<td><strong>$header</strong></td>";
				}
				echo "</tr>".$endl;
				
				$row=mysql_fetch_row($resource);
				while ($row)
				{
					echo "<tr align=center>";
					foreach ($row as $attribute)
					{
						echo "<td>$attribute</td>";
					}
					echo "</tr>".$endl;
					$row=mysql_fetch_row($resource);
				}
				echo "</table>".$endl;
			}
			else
				echo "<p>mySQL Error: <em>".mysql_error()."</em>".$endl;
		}
		else
			echo "Error".$endl;
		mysql_close($mySQL);	
	}
	?>
</body>

</html>
