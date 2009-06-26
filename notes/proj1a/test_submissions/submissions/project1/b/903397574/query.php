<html>
<head><title>SQL Query Emulator: Project 1B</title></head>
<body>
Type an SQL query in the following box:
<p>
<form action="./query.php" method="GET">
<textarea name="query" cols="60" rows="8"><?php if(isset($_GET['query'])){ echo $_GET['query']; } ?></textarea>
<input type="submit" value="Submit" />
</form>
</p>
<p><small>Note: tables and fields are case sensitive. Run "show tables" to see the list of
available tables.</small></p>


<?php
	if(isset($_GET['query'])){
		echo "<h3>Results from MySQL:</h3>";
		
		$con = mysql_connect("localhost","cs143","");
		if(!$con){
			die('Could not connect: ' .mysql_error());
		}
		
		mysql_select_db("CS143",$con);
		$sql = mysql_real_escape_string(trim($_GET['query']));
		
		$result = mysql_query($sql); 
		if(!$result){
			echo mysql_error();
		}
		else{
		
			echo "<table border=1 cellspacing=1 cellpadding=2>";
			
			echo "<tr align=center>";
			while($property = mysql_fetch_field($result)){
				echo "<td><b>".$property->name."</b></td>";
			}
			echo "</tr>";
			

			while($row = mysql_fetch_assoc($result)){
				echo "<tr align=center>";
				foreach($row as $v){
					echo "<td>".($v ? $v : "N/A")."</td>";
				}
				//print_r($row);
				echo "</tr>";
			}
			
			echo "</table>";
		}
	}
?>

</body>
</html>
