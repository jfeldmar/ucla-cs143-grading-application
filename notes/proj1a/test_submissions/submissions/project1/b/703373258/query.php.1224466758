<html>
	<head>
	<title>CS143 Project 1B</title>
	</head>
	<body>
		<p>Type an SQL query in the following box:<br/>
		</p>

			<form "query.php" method="GET">
				<textarea rows="8" cols="60" name="query"></textarea>
				<input type="submit" value="Submit"/>
			</form>

		<p>
			<small>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.</small>
		</p>
		
		<?php

			if($_GET["query"])
			{
				$db_connection = mysql_connect("localhost", "cs143", "");
				mysql_select_db("TEST", $db_connection);
				$query = $_GET["query"];
				$rs = mysql_query($query, $db_connection);
				print "<h3>Results from MySQL:</h3>";
				print "<table cellspacing=\"1\" cellpadding=\"2\" border=\"1\">";
				print "<tbody>";
				print "<tr align=\"center\">";
				$countCol = 0;
				while($countCol < mysql_num_fields($rs)){
					$name = mysql_field_name($rs,$countCol);
					print "<td>";
					print "<b>$name</b>";
					print "</td>";
					$countCol++;
				}
				print "</tr>";

				while($row = mysql_fetch_row($rs)){
					$colCount = 0;
					print "<tr align=\"center\">";
					while($colCount<count($row)){

						if(!$row[$colCount])
							print "<td>N/A</td>";
						else
							print "<td>$row[$colCount]</td>";
						$colCount++;
					}
					print "</tr>";	
				}
				mysql_close($db_connection);
			}
		?>
	</body>
</html>

