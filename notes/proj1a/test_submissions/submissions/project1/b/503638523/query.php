<html>
<head><title>CS143 Project 1B query.php</title></head>
<body>
Type an SQL query in the following box:
<p>
<form action="query.php" method="GET">
<textarea name="query" cols="60" rows="8"></textarea>
<input type="submit" value="Submit" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
available tables.</small></p>

<?php

$string = $_GET["query"];

if($string)
{

//preg_replace("/^ +/","",$string);
//echo $string;
if(!preg_match("/^( *|\n*)(select|show)/i",$string))
{
	echo "Sorry, only SELECT and SHOW queries are allowed!";
}
else{
	echo "<h2>Results from MySQL:</h2>";

	$db_connection = mysql_connect("localhost", "cs143", "");
/*
if(!$db_connection){
    $errmsg = mysql_error($db_connection);
    print "Connection failed: $errmsg <br />;
    exit(1);
}
*/
	mysql_select_db("CS143", $db_connection);
	$result = mysql_query($string, $db_connection);
	if (!$result) {
	    echo 'Could not run query.';
	    exit;
	}

	//mysql_field_name($result, 0);

	print "<table border=1 cellspacing=1 cellpadding=2><tr align=center>\n";

	for($i=0; $i < mysql_num_fields($result); $i++){
		print "<td><b>".mysql_field_name($result, $i)."</b></td>";
	}
	print "</tr>";

	$count=0;
	while($row = mysql_fetch_row($result)) {

		print "\n<tr align=center>";
		for($i=0; $i < mysql_num_fields($result); $i++){
			
			if($row[$i]==NULL){ $row[$i]="N/A"; }
			print "<td><b>".$row[$i]."</b></td>";
		}
	}
	print "\n</tr></table>";

	mysql_close($db_connection);
}//end if_else(!preg_match)
}//end if($string)
?>

</body>
</html>
