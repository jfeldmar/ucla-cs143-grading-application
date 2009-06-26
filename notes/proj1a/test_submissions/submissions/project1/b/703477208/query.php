<html>
<head>
<title>CS143 Project 1B Demo by Tarun</title>
</head>
<body>
Type an SQL query in the following box:
<p> </p>
<form method="get" action="query.php">
   <textarea rows="8" cols="60" name="query"></textarea>
<input type="submit" value="Submit"/>
</form>

   <?php
   //rememember to change the username and password
	if($_GET["query"])
	{
	 $query = $_GET["query"];
   	 $db_connection = mysql_connect("localhost", "cs143", "");
	if (!$db_connection)
	  {
	    die('Could not connect: ' . mysql_error());
	  }

	//remember to change the database name
	mysql_select_db("CS143", $db_connection);

	$rs = mysql_query($query, $db_connection);
	if(!$rs)
	{
		echo "Could not run thequery" . mysql_error();
		exit;
	}
	
	 $number_rows = mysql_num_rows($rs);
	 $number_columns = mysql_num_fields($rs);
	
 //printing tables now
	echo '<table border = "1">';
	echo '<tr>';
	for($i=0; $i < $number_columns; $i++)
	{
		echo '<td>';
		echo mysql_field_name($rs,$i);
		echo '</td>';
	}
	echo '</tr>';
	while($row = mysql_fetch_row($rs)) 
	{
	 	echo '<tr>';
	 	 for($i=0; $i < $number_columns; $i++)
		    {
			echo '<td>';
		        if($row[$i] == NULL)
				print "N/A";
			else
		    		print $row[$i];
			echo '</td>';
		    }
		echo '</tr>';
	}
     	mysql_close($db_connection);
	}
?>

</body>
</html>
