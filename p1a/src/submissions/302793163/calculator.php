<html>
<body>

Calculator<br />
Type an expression in the following box (e.g., 10.5+20*3/25).<br /><br />

<form  action = "calculator.php" method = "post">
<input type = "text" name = "msg" />
<input type = "submit" value = "calculate" />
</form>

<?php

$msg = $_POST["msg"];
$condition = true;

/**
if ( preg_match("/[A-Z`~!@#$%^&()_=,<>?\{\}\|\[\]]/i", $msg) ) {
$condition = false;
}
**/

if ( preg_match("/[^0-9+\-*\/\.]/", $msg) ) {
$condition = false;
}


if ( preg_match("/[+\-*\/\.][ ]*[+\-*\/\.]/", $msg) ) {
$condition = false;
}

if ( !preg_match("/[+\-*\/\.][0-9]/", $msg) ) {
$condition = false;
}

if ( $condition ) {
echo $msg . " = ";
eval("\$msg = $msg;");
echo $msg;
} else {
echo "invalid input";
}

?>

</body>
</html>