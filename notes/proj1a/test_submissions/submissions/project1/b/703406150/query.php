<html>
<body>

<h1> My IMDB </h1>

<h3>Please enter a query:</h3>

<!-- Make a text box -->
<form action="query.php" method="get">
<textarea name="input" cols="50" rows="15">
</textarea>
<input type="submit" value="Query" />
</form>

<?php
// connect to the database
$con = mysql_connect("localhost","cs143","");
if (!$con)
	die('Could not connect: ' . mysql_error());

// use the CS143 database
mysql_select_db("CS143", $con);

// get the input from the form into $q
$q = $_GET["input"];

// query
$result = mysql_query($q, $con);

// if there is no input
if ($q == NULL)
{
	// do nothing
}
else
{
	echo '<b>'."Here is your query:  ".'</b>';
	echo $q;
	echo "\n";
	echo "<table border='2'>
	<tr>
	<th>Field 1</th>
	<th>Field 2</th>
	<th>Field 3</th>
	<th>Field 4</th>
	<th>Field 5</th>
	<th>Field 6</th>
	<th>Field 7</th>
	<th>Field 8</th>
	<th>Field 9</th>
	<th>Field 10</th>
	<th>Field 11</th>
	<th>Field 12</th>
	<th>Field 13</th>
	<th>Field 14</th>
	<th>Field 15</th>
	
	</tr>";

	while($row = mysql_fetch_array($result))
	{
		echo "<tr>";
		for ($i=0; $i < 16; $i++)
		{
			echo "<td>".$row[$i]."</td>";
		}
		echo "</tr>";
	}
	echo "</table>";
}
// close the connection
mysql_close($con);
?>

</body>
</html>
