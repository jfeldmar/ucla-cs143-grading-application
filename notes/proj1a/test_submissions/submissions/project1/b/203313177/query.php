<?php print('<?xml version = "1.0" encoding = "utf-8"?>')?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns = "http://www.w3.org/1999/xhtml">
	<head>
		<title>CS 143 Queries</title>
	</head>
	<body>
		<h3>Type an SQL query in the following box:</h3>
		<form action = "" method = "post">
			<p>
				<textarea name = "queryspace" rows = "20" cols = "50"></textarea>
			</p>
			<p>
				<input type = "submit" name = "submit" value = "Submit" />
			</p>
		</form>
		<?php
			extract ($_POST);
			if (isset($submit))
			{
				$query = $queryspace;
				if (!($db_link = mysql_connect("localhost", "cs143", "")))
					die ("Could not connect to database </body></html>");
				if (!($db_selected = mysql_select_db("CS143", $db_link)))
					die ("Could not open CS143 database </body></html>");
				if (!($result = mysql_query($query,$db_link)))
				{
					print ("Could not execute query! <br />");
					die (mysql_error() . "</body></html>");
				}
				mysql_close($db_link);
				print ("<h3>Query Reults:</h3>");
			}
		?>
		<table border = 1 cellspacing = 1 cellpadding = 2>
		<?php
			if (isset($submit))
			{
				for ($counter = 0; $row = mysql_fetch_row($result); $counter++)
				{
					print("<tr>");
					foreach($row as $key => $value)
						print ("<td>$value</td>");
					print("</tr>");
				}
			}
		?>
		</table>
		<?php
			if (isset($submit))
				print ("<br />Your query returned <strong>$counter results.<br /><br /></strong>");
		?>
	</body>
</html>