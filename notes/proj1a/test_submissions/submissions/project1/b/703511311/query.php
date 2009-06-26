<html>
	<head>
		<title>CS143 Project 1B Demo</title>
	</head>
	<body>
		Type an SQL query in the following box:
		<p>
		<form action="." method="GET">
		<textarea name="query" cols="60" rows="8"></textarea>
		<input type="submit" value="Submit" />
		</form>
		</p>
		<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.</small></p>
		<?php
			function SQLRead($query)
			{
				
				$db_connection = mysql_connect("localhost", "cs143", "");
				
				mysql_select_db("CS143", $db_connection);
				
				$rs = mysql_query($query, $db_connection);
				
				printf("<h3>Results from MySQL:</h3>\n\n");
				printf("<table style='color:white; background:blue' border=1 cellspacing=1 cellpadding=2>\n");
				
				$row = mysql_fetch_row($rs);
				$i = 0;
				printf("<tr align=center style='color:white; background:black'>");
				while ($i < sizeof($row))
				{
					printf("<td>".mysql_field_name($rs,$i)."</td>");
					$i++;
				}
				printf("</tr>\n");
				
				do
				{
					$i = 0;
					printf("<tr align=center>");
					while ($i < sizeof($row))
					{
						if ($row[$i] == null)
						{
							printf("<td>N/A</td>");
						}
						else
						{
							printf("<td>");
							printf($row[$i]);
							printf("</td>");
						}
						$i++;
					}
					printf("</tr>\n");
				}while ($row = mysql_fetch_row($rs));
				printf("</table>\n");
				
				mysql_close($db_connection);
			}

			$q = $_REQUEST["query"];
			if ($q != null)
				SQLRead($q);
				
			
		?>
	</body>
</html>
