<!--
Name: Angel Darquea
ID: 003512936
Date: 10/19/2008
Fall 2008
Project 1B - query.php
-->
<html>
	<head><title>CS143 Project 1B</title></head>
	<font color = blue><h2>THE MOVIE DATABASE</font></h2>
	<body>
		Type an SQL query in the following box:
		<p>
			<form action="./query.php" method="GET">
				<textarea name="query" cols="60" rows="8"></textarea>
				<input type="submit" value="Submit" />
			</form>
		</p>
		<p>
			<small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
		available tables.</small>
		</p>
		
		<!--PHP Script-->
		<?php
			if ($_GET["query"]){
				// Check that the input is a SELECT or SHOW query and execute, return 'Sorry' message if not:
				$queryLike = "/select*|show*/i";
				if ( preg_match($queryLike, $_GET["query"] ) == 1) {
					$myQuery = $_GET["query"];
					//Establish db connection , check it was established:
					$db_connection = mysql_connect("localhost", "cs143", "");
					if (!$db_connection) {
						$errmsg = mysql_error($db_connection);
						echo "Connection failed: $errmsg <br />";
						exit(1);
					}
					// Tell mySQL server which database to use; check if error:
					$connected2db = mysql_select_db("CS143", $db_connection);
					if (!$connected2db) {
						echo "Data base not found!<br />";
						mysql_close($db_connection);
						exit(1);
					}			
					//Send user's query to the database, check if errors; return resulting relation to the resource 'rs', print results:
					echo ("<h3>Results from MySQL:</h3>");
					$rs = mysql_query($myQuery, $db_connection);
					if (!$rs) {
						$errmsg = mysql_error();
						echo ("$errmsg");
						mysql_close($db_connection);
						exit(1);
					}
					// Display table in HTML
					// Get the number of fields from rs:
					$numFields = mysql_num_fields($rs);
					echo ("<table border=1 cellspacing=1 cellpadding=2><tr align=center BGCOLOR=cyan> ");
					// Print the table headers:
					for($i=0; $i<$numFields; $i++)
					{
					    $field = mysql_fetch_field($rs);
					    echo "<td><b>{$field->name}</b></td>";
					}
					echo "</tr>\n";
					// Now print the table records:
					while($row = mysql_fetch_row($rs)) //mysql_fetch_row returns an array
					{
					    echo "<tr align=center>";
					    foreach($row as $cell) // Loop through the elements of the row.
							if (is_null($cell))
								echo "<td>N/A</td>";
							else
								echo "<td>$cell</td>";
					    echo "</tr>\n";
					}
					echo "</table>";
										
					//We are done so we release 'rs''s memore and close the connection:
					mysql_free_result($rs);
					mysql_close($db_connection);
				}
				else
					echo ("Sorry, only SELECT and SHOW queries are allowed!");
			}	
		?>

	</body>
</html>
