<html>
	<head>
		<title>Query Page
		</title>
	</head>

	<body>
		<p>
			Type an SQL query in the following box:
		</p>

		<form action="query.php" method="GET">
			<textarea name="query" cols="60" rows="8"></textarea>
			<input type="submit" value="Submit"/>
		</form>
				
		<?php			
			if($_GET["query"])
			{			

				$db_connection = mysql_connect("localhost", "cs143", "");
				mysql_select_db("CS143",$db_connection);
				
				$query = $_GET["query"];
				$rs = mysql_query($query, $db_connection);
				
				if(!$rs)
				{
					echo mysql_error();
					exit;
				}
				
				echo "<h2> Results from MySQL: </h2> <p>";					

				echo '<table border="1">';

				//lengths is the number of columns returned in result set $rs 
				$lengths = mysql_num_fields($rs);
				
				//First row contains the column name
				echo '<tr align ="center">';
				for($i = 0; $i < $lengths; $i++)
				{
					echo "<td>";
					$field_name = mysql_field_name($rs,$i);
					echo "<b>".$field_name."</b>";
					echo "</td>";
				}
				echo "</tr>";
				//Column names are successfully printed
			
				//Display each row
				while($row = mysql_fetch_row($rs)) 
				{
					echo '<tr align="center">';
					for($i = 0; $i < $lengths; $i++)
					{
						if($row[$i] == NULL)
						{
							echo "<td> N/A </td>";
						}
						else
						{
							echo "<td>".$row[$i] ."</td>";
						}		
					}
					echo "</tr>";
				}
				echo "</table>";
			
				mysql_close($db_connection);
			}
		?>
		
	</body>
</html>