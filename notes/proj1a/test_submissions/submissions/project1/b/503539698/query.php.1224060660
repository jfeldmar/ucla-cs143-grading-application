<html>
<head>
	<title>CS143 Project 1B Demo</title>
</head>
<body>
<p>
<form action="query.php" method="GET">
	<p>
		Select a database:
	</p>
	<input type="radio" name="table_name" value="TEST" checked> TEST
	&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
	<input type="radio" name="table_name" value="CS143"> CS143
	<p>
		Type an SQL query in the following box:
	</p>
	<textarea name="query" cols="60" rows="8"><?php echo trim($_GET['query']); ?></textarea>
	<input type="submit" value="Submit" />
</form>
</p>
<p>
	<small>
		Note: tables and fields are case sensitive.
		Run "show tables" to see the list of available tables.
	</small>
</p>

<?php
	// Catch the user submission
	if ( isset($_GET['query']) && !empty($_GET['query']) )
	{
		// Prep for result
		echo '<h3>Results from MySQL:</h3>';
		
		// Normalize the user input
		$query = trim($_GET['query']);
		$table_name = ( isset($_GET['table_name']) ) ? $_GET['table_name'] : 'TEST';
		
		// MySQL Connection
		$db_connection = mysql_connect('localhost', 'cs143', '');
		
		// Handle Connection Errors
		if ( !$db_connection ) {
			$error_message = mysql_error($db_connection);
			print 'Connection failed: ' . $error_message . '<br />';
			exit(1);
		}
		
		// Select the Database
		mysql_select_db($table_name, $db_connection);
		
		// Issue the query
		$rs = mysql_query($query, $db_connection);
		
		// Open the table
		echo '<table border=1 cellspacing=1 cellpadding=2>';
		
		// Output the Column names
		echo '<tr align=center>';
		for($i = 0; $i < mysql_num_fields($rs); $i++) {
			// Get Column Meta Data
			$meta = mysql_fetch_field($rs, $i);
			// Output the column name
			echo '<td><b>'.$meta->name.'</b></td>';
		}
		echo '</tr>';
		
		// Retrieve results, row by row
		while( $row = mysql_fetch_row($rs) ) {
			echo '<tr align=center>';
			// Go through each column
			for($i = 0; $i < mysql_num_fields($rs); $i++) {
				$cell = (empty($row[$i])) ? 'N/A' : $row[$i];
				echo '<td>' . $cell . '</td>';
			}
			echo '</tr>';
		}
		
		// Close the table
		echo '</table>';
		
		// Close the MySQL Connection
		mysql_close($db_connection);
	}
?>
</body>
</html>
