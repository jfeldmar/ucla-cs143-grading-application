<html>

  <head>
    <title>CS143 Project 1B</title>
  </head>

  <body>

    <form action="query.php" method="GET">
    <textarea name = "query" cols = "50" rows = "10"></textarea>
    <input type = "submit" value="Submit">
    </form>
    <br />

    <?php

      if($_GET["query"])
      {
        $db_connection = mysql_connect("localhost", "cs143", "");

	mysql_select_db("CS143", $db_connection);

	$query = $_GET["query"];

	$rs = mysql_query($query, $db_connection);
	
	echo "Results from MySQL:";

	echo "<table border = \"1\">";
	echo "<tr>";
	
	echo "<tr>";

	for($j = 0; $j < mysql_num_fields($rs); $j++)
	{
	  echo "<th>";
	  echo mysql_field_name($rs,$j);
	  echo "</th>";		 
        }

	echo "</tr>";

	while($row = mysql_fetch_row($rs)) 
	{
	  echo "<tr>";

	  for($j = 0; $j < mysql_num_fields($rs); $j++)
	  {
	    echo "<td>";
	    echo "$row[$j]";
	    echo "</td>";		 
	  }

	  echo "</tr>";
	}

	mysql_close($db_connection);
      }

    ?>
 
  </body>

</html>