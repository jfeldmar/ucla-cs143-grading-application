<html>

<head>
<title>
CS143 Project 1B
</title>
</head>

<body>
Type a SQL query in the following box:

<form method="post">
<textarea name="query" cols="100" rows="10">
</textarea>
<input type="submit" value="Submit" />
</form>

<h2>Results from MySQL:</h2>

<?php
  // request the input query
  $query = $_REQUEST["query"];
  
  if ($query != "") {
    // establish db connection
    $db_connection = mysql_connect("localhost", "cs143", "");
    // db connection error handling
    if(!$db_connection) {
      $errmsg = mysql_error($db_connection);
      echo "Connection failed: " . $errmsg . "<br />";
      exit(1);
    }
    // select database CS143
    mysql_select_db("CS143", $db_connection);

    // run the query in db connection
    $rs = mysql_query($query, $db_connection);
    
    // get query's number of rows
    $col = mysql_num_fields($rs);
    
    print "<table border = 3>";
    // field names
    $k = 0;
    print "<tr>";
    while ($k < $col) {
      // get the field name
      $attr = mysql_field_name($rs, $k);
      print "<td>";
      print "$attr";
      print "</td>";
      $k++;
    }
    print "</tr>";
    
    // fetch the data
    while ($data = mysql_fetch_row($rs)) {
      // row index
      $i = 0;
      print "<tr>";
      while ($i < $col) {
        print "<td>";
        print "$data[$i]";
        print "</td>";
        // decrement the row
        $i++;
      }
      print "</tr>";
    }
    print "</table>";

    // close db connection
    mysql_close($db_connection);
  }
?>
</body>

</html>

