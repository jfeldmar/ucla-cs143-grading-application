<?php
$dbh=mysql_connect ("localhost", "cs143", "") or die ('I cannot connect to the database because: ' . mysql_error() . '<br>The server is currently down.  Please come back another time.');
mysql_select_db ("CS143", $dbh);
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>CS143 Project 1B</title>
</head>
<body>
Type an SQL query in the following box:
<p>
<form action="" method="post">
<textarea name="query" cols="60" rows="8"><? echo $_POST['query']; ?></textarea>
<input type="submit" value="Submit" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
available tables.</small></p>
<?
if ($_POST['query']){
	echo "<h3>Results from MySQL:</h3>";
	$query=$_POST['query']; 
	$sanitized_name = mysql_real_escape_string($name, $dbh);
	$query_to_issue = sprintf($query, $sanitized_name);
	$rs = mysql_query($query_to_issue, $dbh) or die(mysql_error());
	if (preg_match('/(SELECT|SHOW)/',$query,$match)){
		echo "<table border=1 cellspacing=1 cellpadding=2>";
		while($row = mysql_fetch_row($rs)) {
			echo "<tr align=center>";
			foreach ($row as $value) {
				echo "<td>$value</td>";
			}
			echo "</tr>";
		}
		echo "</table>";
	} else {
		$affected = mysql_affected_rows($dbh);
		print "Total affected rows: $affected<br />";
	}
}
?>
</body>
</html>
<? mysql_close($dbh); ?>
