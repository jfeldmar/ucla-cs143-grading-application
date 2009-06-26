<html>
	<head>
		<title>Javier Rey's CS143 Project 1B</title>
	</head>
	<body>
		Type an SQL query in the following box:
		<form method="GET">
			<textarea name="query" cols="60" rows="8"><?php 
				if($_GET["query"]) {
					echo $_GET["query"];
				}
			?></textarea>
			<input type="submit" value="Submit" />
		</form>
		<?php
			if($_GET["query"]) {
				// Establishing a connection to the database system
				$db_connection = mysql_connect("localhost", "cs143", "");
				if(!$db_connection) {
    				$errmsg = mysql_error($db_connection);
    				print "Connection failed: $errmsg <br />";
   					exit(1);
				}
				
				// Select the CS143 database
				mysql_select_db("CS143", $db_connection);
				
				// Sanitize user input
				$query = mysql_real_escape_string($_GET["query"], $db_connection);
				
				// Get resource containing the results of the query
				$rs = mysql_query($query, $db_connection);
				
				// Display table with results
				echo "<h3>Results from MySQL:</h3>\n";
				echo "<table border=1 cellspacing=1 cellpadding=2>\n";
				// Display table attributes
				echo "	<tr align=center>\n";
				$num_attr = mysql_num_fields($rs);
				for($i = 0; $i < $num_attr; $i++) {
					$attr_name = mysql_field_name($rs, $i);
					echo "		<td><b>$attr_name</b></td>\n";
				}
				echo "	</tr>\n";
				// Display tuples
				while($row = mysql_fetch_row($rs)) {
					echo "	<tr align=center>\n";
					for($i = 0; $i < $num_attr; $i++) {
						$attr_value = $row[$i];
						if($attr_value) {
							echo "		<td>$attr_value</td>\n";
						} else {
							echo "		<td>N/A</td>\n";
						}
					}									
					echo "	</tr>\n";
				}
				echo "</table>\n";
				
				// Close connection to the database
				mysql_close($db_connection);
			}
		?>
	</body>
</html>
