<html>

<head>
  <title></title>
</head>

<body>

<h1>Movie Database</h1>

<form action="query.php" method="GET">
	<textarea name="query" rows=5 cols=50></textarea>
	<br />
	<input type="submit" />
	<br />
</form>

<h2>Query Result</h1>

<?php
	$db_connection = mysql_connect("localhost", "cs143", "");
        if(!$db_connection) {
                $errmsg = mysql_error($db_connection);
    		print "Connection failed: $errmsg<br />";
   		exit(1);
        }

	$db_select = mysql_select_db("TEST", $db_connection);
        if(!$db_select) {
                $errmsg = mysql_error($db_connection);
    		print "DB selection failed: $errmsg<br />";
   		exit(2);
        }

	$query = $_GET["query"];
	$strSegment = explode(" ", $query, 2);

	if(!$query) {
        	echo "Please enter a SELECT or SHOW query.";
	}
	else if(!strcasecmp($strSegment[0], "select") || !strcasecmp($strSegment[0], "show")) {
		$rs = mysql_query($query, $db_connection);
                if(!$rs) {
                        $errmsg = mysql_error($db_connection);
    			print "Query failed: $errmsg<br />";
   			exit(3);
                }

        	$numCol = mysql_num_fields($rs);

        	print "<table border=1>";
        	print "<TR>";

	        for($i = 0; $i < $numCol; ++$i) {
        		print "<TH align=center>".mysql_field_name($rs, $i);
	        }

        	while($row = mysql_fetch_row($rs)) {
                	print "<TR>";
        		for($i = 0; $i < $numCol; ++$i) {
                        	print "<TD align=center>".$row[$i];
                	}
        	}

		print "</table>";
	}
        else {
        	echo "Only SELECT and SHOW queries are allowed!!";
        }

	mysql_close($db_connection);
?>

</body>

</html>

