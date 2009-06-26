<!-- Tanya Gillis CS 143 Project 1B -->
<!-- query.php -->

<html>
<body>

Type an SQL query in the following box: </br>

<form action="query.php" method="get">
<TEXTAREA name="query" ROWS=8 COLS=60>
</TEXTAREA>
<input type="submit" value="Submit"/>
</form>

<font size = "2"> Note: Tables and fields are case sensitive. Run "show tables" to see the list of available tables.<br/> </font>

<?php

$db_connection = mysql_connect("localhost", "cs143", "");
mysql_select_db("CS143", $db_connection);

if(!db_connection)
{ 
  $errmsg = mysql_error($db_connection);
  print "Connection failed: $errmsg <br />";
  exit(1);
}

$query = $_GET["query"];
$sanitized_name = mysql_real_escape_string($name, $db_connection);
$query_to_issue = sprintf($query, $sanitized_name);
$rs = mysql_query($query_to_issue, $db_connection);

echo "</br> <h3>Results from MySQL: <h3>";

echo "<table border = \"1\" align = \"left\">";
echo "<tr>";

$i = 0;

while($col = @mysql_fetch_field($rs))
{
  $cn = mysql_field_name($rs, $i);
  echo "<th> $cn </th>";
  $i = $i + 1;
}
echo "</tr>";

$num = @mysql_num_fields($rs);

while ($row = @mysql_fetch_row($rs))
{
  $j = 0;
  
  echo "<tr>";
  while ($j < $num)
  {
    if ($row[$j] == NULL)
      echo "<td>N/A</td>";
    else
      echo "<td>$row[$j]</td>";
    $j = $j + 1;
  }
  echo "</tr>";

}
echo "</table>";

mysql_close($db_connection);

?>

</body>
</html>
