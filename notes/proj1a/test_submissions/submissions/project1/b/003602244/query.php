<html>
<head><title>Query.php</title></head>
<body>

<font><B>Type an SQL query in the following box:</B></font>
<br>

<form method = "POST" action="query.php">
<TEXTAREA NAME="area" ROWS=5 COLS=30>
</TEXTAREA>
<input type="submit">
</form>
<br>
Note: tables and fields are case sensitive.  Run "Show tables" to see the list of avaliable tables.
<br><br>

<?php

// connect to mysql 
$db_connection = mysql_connect("localhost", "cs143", "");

// error msg
if(!$db_connection) {
    $errmsg = mysql_error($db_connection);
    echo "Connection failed: ".$errmsg;
    exit(1);
}

// select database
mysql_select_db("CS143", $db_connection);

// handling user input
$query = $_POST["area"];
$sanitized_name = mysql_real_escape_string($name, $db_connection);
$query_to_issue = sprintf($query, $sanitized_name);
$rs = mysql_query($query_to_issue, $db_connection);

if(mysql_num_rows($rs) == 0) {
	echo "No Result \n";
	mysql_error();
	exit(1);
}else{
	while($row = mysql_fetch_row($rs)) {
		$r0 = $row[0];
		$r1 = $row[1];
		$r2 = $row[2];
		$r3 = $row[3];
		$r4 = $row[4];
		$r5 = $row[5];
		$r6 = $row[6];
		$r7 = $row[7];
		$r8 = $row[8];
		$r9 = $row[9];
		echo "$r0 $r1 $r2 $r3 $r4 $r5 $r6 $r7 $r8 $r9 <br />";
	}
}

// close mysql connection
mysql_close($db_connection);

?>

</body>
</html>
