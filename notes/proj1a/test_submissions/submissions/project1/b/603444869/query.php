<?php
$query = $_POST["query"];
if ($query) {
echo "query: $query";
$x = mysql_connect('localhost','cs143',''); 
mysql_select_db("CS143",$x);
$s = mysql_query($query,$x);
$num = mysql_num_fields($s);
echo '<table border=1>';
for ($i=0; $i<$num; $i++) {
    $col[] =  mysql_field_name($s, $i);
    echo '<th>'.$col[$i].'</th>';
}
while($row=mysql_fetch_array($s)) {
    echo '<tr>';
    for ($j=0; $j<$num; $j++) {
        if ($row[$col[$j]]) {
            echo '<td>'.$row[$col[$j]].'</td>';
        } else {
            echo '<td>NULL</td>';
        }
    }
    echo '</td>';
}
echo '</table>';
echo "<br>";
echo "<hr>";
mysql_close($x);
}

?>
<html>
<head><title>CS143 Project 1B</title></head>
<body>
Type an SQL query in the following box:
<p>
<form method="POST" ACTION="<?php echo $PHP_SELF;?>">
<textarea name="query" cols="60" rows="8"></textarea>
<input type="submit" value="Submit" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
available tables.</small></p>


</body>

</html>

