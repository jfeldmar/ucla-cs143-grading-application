<html>
  <head>
    <title>Barebones Movie Database Query Interface</title>
    <style type="text/css">
      table {text-align:center; border: 0px; border-spacing: 0px}
      body {font-family: Arial}
    </style>
  </head>
  <body>
    <p>Type an SQL query in the following box:</p>
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="GET">
      <textarea rows="10" cols="60" name="query_string"><?php echo $_REQUEST['query_string']?></textarea>
      <input type="submit" value="Submit" />
    </form>
    <br />
    <?php
      $qs = $_GET["query_string"];
      if ($qs) {
        if (!preg_match("/^ *select.*$/im", $qs) && !preg_match("/^ *show.*$/im", $qs)) {
          echo 'Only SELECT and SHOW queries are allowed!<br />';
            exit(1);
        }
        $conn = mysql_connect("localhost", "cs143", "");
        if (!$conn) {
          $errmsg = mysql_error($conn);
          echo 'Connection failed: ' .$errmsg .'<br />';
          exit(1);
        }
        $db = mysql_select_db("CS143", $conn);
        if (!$db) {
          $errmsg = mysql_error($conn);
          echo 'Selecting database failed: ' .$errmsg .'<br />';
          exit(1);
        }
      
        $rs = mysql_query($qs, $conn);
        if (!$rs) {
          $errmsg = mysql_error($conn);
          echo 'Query failed:<br /> ' .$errmsg;
          exit(1);
        }
        $num_fields = mysql_num_fields($rs);
        if (!$num_fields) {
          $errmsg = mysql_error($conn);
          echo 'Error: ' .$errmsg .'<br />';
          exit(1);
        }
        echo '<table border="1"><tr>';
        for ($i = 0; $i < $num_fields; $i++) {
          echo '<th>' .mysql_field_name($rs, $i) .'</th>';
        }
        echo '</tr>';
        while($r = mysql_fetch_row($rs)) {
          echo '<tr><td>' .implode($r, '</td><td>') .'</td></tr>';
        }
        echo '</table>';
      }
    ?>
  </body>
</html>
