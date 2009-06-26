<HTML>

<HEAD>
	<TITLE>CS143 Project 1B</TITLE>
<HEAD>

<BODY>
	<P>Type an SQL query in the following box:</P>
	<FORM ACTION="query.php" METHOD="GET">
		<TEXTAREA ROWS="8" COLS="60" NAME="query"><?PHP if ($_GET["query"]) echo $_GET["query"]; ?></TEXTAREA>
		<INPUT TYPE="SUBMIT" VALUE="Submit" />
	</FORM>
	<SMALL>Note: tables and fields are case sensitive. Run "show tables" to see the list of available tables.</SMALL>

<?PHP
	function check_error_mysql($check, $db) {
		if (!$check) {
			echo "<P>" . mysql_error($db) . "</P>";
			exit(1);
		}
	}

	if ($_GET["query"]) {
		echo "<H3>Results from MySQL:</H3>\n";
		$db = mysql_connect("localhost", "cs143", "");
		mysql_select_db("CS143");
		$query = mysql_real_escape_string($_GET["query"]);
		$result = mysql_query($query, $db);
		check_error_mysql($result, $db);
		$num = 0;
		while ($row = mysql_fetch_row($result)) {
			check_error_mysql($row, $db);
			if ($num == 0) {
				echo "<TABLE BORDER=\"1\">\n";
				echo "<TR>\n";
				for ($r = 0; $r < mysql_num_fields($result); $r++) {
					$name = mysql_field_name($result, $r);
					check_error_mysql($name, $db);
					echo "<TD ALIGN=\"CENTER\"><B>$name</B></TD>";
				}
				echo "</TR>\n";
			}
			echo "<TR>";
			foreach ($row as $col) {
				echo "<TD ALIGN=\"CENTER\">$col</TD>";
			}
			echo "</TR>";
			$num++;
		}
		if ($num != 0) echo "</TABLE>\n";
		mysql_close($db);
	}
?>

</BODY>

</HTML>
