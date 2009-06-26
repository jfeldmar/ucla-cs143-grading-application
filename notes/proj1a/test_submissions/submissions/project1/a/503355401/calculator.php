<?PHP session_start() ?>

<HTML>

<HEAD>
	<TITLE>CS143 Calculator</TITLE>

	<STYLE TYPE="text/css">
		.error {color: #ff0000}
		.button_small {width: 2em; font-size: 1em; font-weight: bold}
		.button_large {width: 4em; font-size: 1em; font-weight: bold}
		.display {width: 8em; font-size: 1em; font-family: monospace; text-align: right}
	</STYLE>

	<SCRIPT LANGUAGE="javascript">
		function display_clear() {
			document.getElementById('display').value = "";
		}
		function display_del() {
			document.getElementById('display').value = document.getElementById('display').value.substr(0, document.getElementById('display').value.length - 1);
		}
		function display_append(text) {
			document.getElementById('display').value = document.getElementById('display').value + text;
		}
	</SCRIPT>

</HEAD>

<?PHP
	function form_draw($display) {
		echo '
<FORM NAME="input" action="calculator.php" method="get">
	<INPUT TYPE="text" CLASS="display" NAME="expr" ID="display" VALUE="' . $display . '"></INPUT>
	<BR />
	<INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'7\')" VALUE="7" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'8\')" VALUE="8" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'9\')" VALUE="9" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'+\')" VALUE="+" />
	<BR />
	<INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'4\')" VALUE="4" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'5\')" VALUE="5" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'6\')" VALUE="6" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'-\')" VALUE="-" />
	<BR />
	<INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'1\')" VALUE="1" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'2\')" VALUE="2" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'3\')" VALUE="3" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'*\')" VALUE="*" />
	<BR />
	<INPUT TYPE="button" CLASS="button_large" ONCLICK="display_append(\'0\')" VALUE="0" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'.\')" VALUE="." /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_append(\'/\')" VALUE="/" />
	<BR />
	<INPUT TYPE="button" CLASS="button_small" ONCLICK="display_clear()" VALUE="AC" /><INPUT TYPE="button" CLASS="button_small" ONCLICK="display_del()" VALUE="&larr;" /><INPUT TYPE="submit" CLASS="button_large" VALUE="=" />
</FORM>
		';
	}

	function history_draw($history) {
		echo "<P>\n";
		echo "<UL>\n";
		if ($history[0] == "") echo "<LI>No history items to display. Evaluate an expression to add it to the history.</LI>";
		foreach ($history as $item) {
			if ($item != "") echo "<LI><A ONCLICK=\"display_clear(); display_append('$item');\">$item</A></LI>\n";
		}
		echo "</UL>\n";
		echo "<A HREF=\"?clear=1\">Clear history</A>\n";
		echo "</P>\n";
	}

	function error($string) {
		echo "\n<P CLASS=\"error\">$string</P>\n</BODY>\n\n</HTML>\n";
		exit(1);
	}

	function expr_parse($delimiter, $expr) {
		$split = strpos($expr, $delimiter);
		$parts[0] = substr($expr, 0, $split);
		$parts[1] = substr($expr, $split + 1, strlen($expr) - $split);
		return $parts;
	}

	function expr_eval($expr) {
		if (is_numeric($expr)) return $expr;
		// Handle operators in order of precedence
		list($left, $right) = expr_parse("-", $expr);
		if ($left != "" && $right != "")
			return expr_eval($left) - expr_eval($right);
		list($left, $right) = expr_parse("+", $expr);
		if ($left != "" && $right != "")
			return expr_eval($left) + expr_eval($right);
		list($left, $right) = expr_parse("/", $expr);
		if ($left != "" && $right != "") {
			$temp = expr_eval($right);
			if ($temp == 0) return "ERR DIV 0";
			return expr_eval($left) / $temp;
		}
		list($left, $right) = expr_parse("*", $expr);
		if ($left != "" && $right != "") {
			$a = expr_eval($left);
			$b = expr_eval($right);
			return expr_eval($left) * expr_eval($right);
		}
		return "ERR SYNTAX";
	}

	if ($_GET["expr"] != "") {
		$expr = $_GET["expr"];
		$_SESSION["history"] .= "$expr&";
		$result = expr_eval($expr);
	}

	if ($_GET["clear"] == "1") $_SESSION["history"] = "";
?>

<BODY>

<H1>CS143 Calculator</H1>

<P>This calculator can be used to perform basic math operations. Type an expression or use the calculator buttons to input an expression. Click "&larr;" if you make a mistake to delete the last character input. Click "AC" if you want to clear everything and start over again.</P>

<H2>Calculator</H2>

<?PHP form_draw($result); ?>

<H2>History</H2>

<P>Click on a history item to insert it into to calculator. It will overwrite any input currently being displayed. Click "Clear history" at any time to clear the contents of the history&mdash;you will not be able to recover history items after they are cleared!</P>

<?PHP history_draw(explode("&", $_SESSION["history"])); ?>

<HR />

<P>Page generated <?PHP echo date("c"); ?></P>

</BODY>

</HTML>
