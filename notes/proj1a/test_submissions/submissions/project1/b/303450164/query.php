<html>
<title>KIMBERLY HSIAO: PROJECT 1B</title>
<body>

<?php 
$conn = mysql_connect(localhost,cs143,"");
if (!$conn)
    die('Could not connect: ' . mysql_error());

mysql_select_db("CS143", $conn);
echo "Type an SQL query in the following box: <br/><br/>";
?>

<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="post">
<textarea cols="60" rows="8" name="query"/><?php echo $_POST['query'];?></textarea>
<input type="submit" value="Submit" name='sub'/>
</form>

<?php if(isset($_POST['sub']))
{
	$inputeq = $_POST['query'];
	echo "<br/>Results from MySQL <br/><br/>";
	
	$rs = mysql_query($inputeq, $conn);
	if (!$rs)
		{die('Invalid query: ' . mysql_error());}
	
//***********************DISPLAY THE HEADINGS***************************************************
	echo "<table border='1'><tr>"; 
	$i = 0;
    while ($i < mysql_num_fields($rs))
    {
       echo "<th>". mysql_field_name($rs, $i) . "</th>";
       $i++;
    }
    echo "</tr>"; 
//**************************DISPLAY RESULTS***********************************************
	
	while ($row = mysql_fetch_array($rs,MYSQL_ASSOC))
    {
		echo "<tr>";
		foreach ($row as $col)
		{
			if(is_null($col))
				echo "<td align='center'>N/A</td>";
			else
				echo "<td align='center'>". $col . "</td>";
		}
    }
}?>

</body>
</html>