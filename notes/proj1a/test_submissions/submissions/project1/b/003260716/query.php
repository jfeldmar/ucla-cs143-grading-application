<html>
<head><title>CS143 Project 1B Demo</title></head>
<body>
Type an SQL query in the following box:
<p>
<form method="GET">
<textarea name="query" cols="60" rows="8"></textarea>
<input type="submit" value="Submit" />
</form>
</p>

<?php
	$db_connection = mysql_connect("localhost","cs143","");
	if (!$db_connection)
	{
  	die('Could not connect: ' . mysql_error());
  }
  if (!mysql_select_db("CS143", $db_connection))
  {
  		$errmsg = mysql_error($db_connection);
			mysql_close($db_connection);
   		die('Could not select DB: ' . $errmsg);
  }
 
	if($_GET["query"])
	{	
		$get = $_GET["query"];
		echo "<h3>Results From MySQL:</h3>";
		$rs = mysql_query($get, $db_connection);
		if (!$rs)
		{
			$errmsg = mysql_error($db_connection);
			mysql_close($db_connection);
   		die($errmsg);
		}
	
		$numfields = mysql_num_fields($rs);
		print("<table border=1 cellspacing=1 cellpadding=2>");
		print("<tr align=center>");
		for ($i=0; $i < $numfields; $i++)
			print("<td><b>".mysql_field_name($rs, $i)."</b></td>");
		print("</tr>\n");
		
		while($row = mysql_fetch_row($rs)) 
		{
			print("<tr align=center>"); 
			foreach($row as $rowi)
			{
				if ($rowi == NULL)
					print("<td>N/A</td>");
				else	  
					print("<td>$rowi</td>");	
			}
			print("</tr>\n");
		}
	}
	mysql_close($db_connection);
?>
</body>

</html>