<!-- 
    Document   : query.php
    Created on : Oct 18, 2008, 2:15:53 PM
    Author        : cody prestwood   303596543
-->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
  <head>
      <title>MySQL Linux - Project 1B</title>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
  </head>
  <body>
  <big style="font-size:22pt; color:aqua">
      MySQL Query for CS143 Database: </big><br><br>
   <form  method="GET">
    Enter an SQL query in the following box:
    <P>
	<!--input type="text" name="calcstr" size="20"/>
          <Age: <input type="text" name="age" /-->
	<TEXTAREA id="Textarea1" name="calcstr" rows="5" cols="43"></TEXTAREA>
    <input type="submit" value="Submit Query" >
    </P><br>
   </form>
    
   <br><br>
   <big style="font-size:22pt; color:aqua">Result</big><br>
    <?php
    if($_GET["calcstr"])
    {
	    $calcin = $_GET["calcstr"];
		print "calcin=".$calcin."<br>";
	    if (!$link = mysql_connect("localhost", "cs143", "")) 
		{
		    echo 'Could not connect to mysql';
		    exit;
		}
		if (!mysql_select_db("CS143", $link)) {
		    echo 'Could not select database';
			echo "MySQL Error: " . mysql_error($link);
			print "<br>";
		    exit;
		}
		// eliminate the lf,cr,etc and escape \133..\140 is [\]^_`
		// query should not end with   ";"
		//$pattern[0]='/(\;).$/m';
		$pattern='/[\012\013\014\015]/';
		//$pattern[1]='/[\073]{1}$/';
		//print "pattern=".$pattern[0]." ".$pattern[1]."<br>";
		$calcin = preg_replace($pattern, ' ', $calcin);
		//print "calcin=".$calcin."<br>";
		$sql = addcslashes($calcin, "\0..\37!@\@\177..\377");
		$decoded = stripcslashes($sql);
		$slashN = strstr($sql,"\134\116");
		if (!empty($slashN)) $sql = $decoded;
		print "sql=".$sql."<br>";
		print "decoded=".$decoded."<br>";
		
		// Now submit the query to sql
		$result = mysql_query($sql, $link);
		if (!$result) {
		    echo "DB Error, could not query the database\n";
		    echo "MySQL Error: " . mysql_error($link);
			print "<br>";
		    exit;
		}

		// returns false if query failed or the number of records for SELECT operation
		
		$nrows = mysql_affected_rows($link);
		if ($nrows < 0) {
			$nrows = mysql_num_rows($result);
		}
		print "<P>Total records: {$nrows} </P>";
		
		if ($nrows > 0)
	    {	
			// setup html table stuff
			print '<table id="Table3" align="left" border="1" cellspacing="2"> <tbody>';
			// loop through the column headers and present them
			$fields=mysql_num_fields($result);
			echo "<tr>"; 
			for ($i=0; $i < $fields; $i++) {
				//Table Header
				print "<th>".mysql_field_name($result, $i)."</th>"; 
			}
			echo "</tr>\n";
			// loop through each record and present them
			while ($row = mysql_fetch_row($result)) { 
				//Table body
				echo "<tr>";
			    for ($f=0; $f < $fields; $f++) {
				//print "row{$f}=".$row[$f]."<br>";
				if (empty($row[$f])) $row[$f]="Null";
			    echo "<td>".$row[$f]."</td>";
				}
				echo "</tr>\n";
			}
			print "</tbody></table>";
	    }
		mysql_close($link);
	}
    ?>    
  </body>
</html>