<html>
<head>
	<title>Computer Science 143 Project 1b Query.php</title>
</head>
<body>

Type in SQL in the following box:<br><br>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="GET">
	<TEXTAREA NAME="query" ROWS=10 COLS=50></TEXTAREA><br>
	<small>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.</small>
	<P><INPUT TYPE=SUBMIT VALUE="submit" /></P>
</form>

<?php 
	$query = $_GET["query"];
	$db_connection = mysql_connect("localhost", "cs143", "");
	mysql_select_db("CS143", $db_connection);
	$pattern = "/^([Ss][Ee][Ll][Ee][Cc][Tt]|[Ss][Hh][Oo][Ww])/";
	$display = false;
	if($query != "") {
		if(preg_match($pattern,$query)) {
			$display=true;
			$rs = mysql_query($query,$db_connection);
		}
		else
			print "Sorry, only SELECT and SHOW queries are allowed!";
	}
?>
<TABLE><TR><TD>
	<?php
	displayResultTable($rs,$display);
	function displayResultTable($rs,$display) {
		if($display == true) {
			$num_col = mysql_num_fields($rs);
			print("<br><BIG><B>Results from MySQL:</B></BIG><br>");
			print("<TABLE border=1 >\n");
			print("<TR>");
			for ($col_num = 0; $col_num < $num_col; $col_num++)
			{
				$field_name = mysql_field_name($rs, $col_num);
				print("<TH>$field_name</TH>");
			}
			print("</TR>\n");
			
			while ($row = mysql_fetch_row($rs))
			{
				print("<TR>");
				for ($col_num = 0; $col_num < $num_col; $col_num++)
					print_row($row[$col_num]);
				print("</TR>\n");
			}
			print("</TABLE>\n");
		}
	}
		
	function print_row($string) {
		if($string == "")
			$string="N/A";
		print ("<TD><center>$string</center></TD>\n");
	}
	?>
</TD></TR></TABLE>

</body>
</html>