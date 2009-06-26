<html>

<form action="query.php" method="GET">


Type an SQL query in the following box: 
<br>
<br>
<TEXTAREA type=text name=userfield COLS=40 ROWS=6></TEXTAREA>
<input type = submit name=submit value = 'Submit'>
<br>
<font size=2>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.</font>

<?php
error_reporting(0);

$formula = $_GET['userfield'];

$db_connection = mysql_connect("localhost", "cs143", "");
mysql_select_db("CS143", $db_connection);
$query = $formula;
$rs = mysql_query($query, $db_connection);

if($_GET['submit'] && $formula != null)
{
?>
	<table border = 1 > <tr> 
	
<?php
	print "<br><br><br>";
	print "<b>Results from MySQL:</b>";
	
	$i = 0;
	while(mysql_field_name($rs, $i))
	{
		$test = mysql_field_name($rs, $i);
		print "<th>$test</th>";
		$i = $i + 1;
	}
?>

	</tr>

<?php 
	while($row = mysql_fetch_row($rs)) {
		$a = $row[0];
		print "<tr><td>$a";
    		if($row[1])
		{
			$b = $row[1];
			print "<td>$b";
		}
    		if($row[2])
		{
			$c = $row[2];
			print "<td>$c";
		}
		if($row[3])
		{
			$d = $row[3];
			print "<td>$d";
		}
		if($row[4])
		{
			$e = $row[4];
			print "<td>$e";
		}
		if($row[5])
		{
			$f = $row[5];
			print "<td>$f";
		}
	}

	print "</tr>";
	
?>
	</table>
<?php


	mysql_close($db_connection);
}

?>

</FORM>
</html>