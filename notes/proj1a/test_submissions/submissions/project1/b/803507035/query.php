<HTML>
<BODY>
<H1>Welcome to Erick's IMDB</H1>
Type a SQL query in the following box:

<form action="query.php" method="get">
<textarea name="u_input" rows=10 cols=60> </textarea>
<input type="submit" value="Submit" />
</form>

<?php 
	error_reporting(0);
	
	if($_GET["u_input"])
	{
		$input=$_GET["u_input"];
		
		//Establish Connection 
		$db_connection = mysql_connect("localhost", "cs143", "");
		if(!$db_connection)
		{
    		$errmsg = mysql_error($db_connection);
    		print "Connection failed: $errmsg <br />";
    		exit(1);
		}
	
		//Select Database CHANGE TO CS143 DATABASE LATER!
		mysql_select_db("CS143", $db_connection);
		
		//Issue Query
		$query = $input;
		$rs = mysql_query($query);
		if (!$rs)
		{
			echo "Sorry, only SELECT and SHOW queries are allowed!";
		}
		else {
			echo "<h3>Reslts from MySQL:</h3>";	
			//Print field titles
			$i = 0;
			echo "<table border='1' CELLPADDING=2 STYLE='font-size:15px'>";
		
			while ($i < mysql_num_fields($rs)) {
				$data = mysql_fetch_field($rs, $i);
				echo "<td>";
				echo "<b>$data->name</b>";
				echo "</td>";
				$i++;
			}
		
			//Print results
			while ($row = mysql_fetch_row($rs)) {
				echo "<tr>";
				$j=0;
				while($j < mysql_num_fields($rs)) {
        			if (is_null($row[$j])){
        				$row[$j]="N\A";
        			}
					echo "<td>";
        			echo "$row[$j]";
        			echo "</td>";
        			$j++;
				}
				echo "</tr>";
			}
		echo "</table>";
		}
		// Close Connection
		mysql_close($db_connection);
	}
?>

</BODY>
</HTML>