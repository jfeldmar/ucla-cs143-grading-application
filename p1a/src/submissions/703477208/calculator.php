<html>
<head>
<title>Calculator</title>
</head>
<body>

<h1>Calculator</h1>
<p>Type an expression in the following box (e.g., 10.5+20*3/25).</p>
<p>  </p>
<form method="get" action="calculator.php">
<input type="text" name="data"/>
<input type="submit" value="Calculate"/>
</form>
<?php
   $subject= $_GET["data"];
$pattern='/^-?[0-9]*([0-9]\.|\.[0-9]|[0-9])[0-9]*([\/*+-]-?[0-9]*([0-9]\.|\.[0-9]|[0-9])[0-9]*)*$/x';
$pattern2='/^$/';
set_error_handler(create_function('$x, $y', 'throw new Exception($y, $x);'), E_ALL & ~E_NOTICE);
function check_syntax($code) {
    return @eval('return true;' . $code);
}
//echo "<h2>Result</h2>";
//echo "<br/>";

$code="echo $subject;";
if(  preg_match($pattern,$subject) && check_syntax($code))
  {
    echo "<h2>Result</h2>";
    try{
      echo $subject;
      echo "=";
      eval("echo $subject;"); 
      echo "<br/>";
    }
    catch(Exception $e)
      {
	echo "<br/>Invalid Inputs. Check your inputs. Please 
remember that due to 09 etc being treated as octal, these values present a problem that could not be fixed with given tools <br/> ";
      }
  }
else
  {
    if(preg_match($pattern2,$subject))
      {
      }
    else{
       echo "<h2>Result</h2>";
      echo "<br/>Invalid Inputs. Check your inputs. Please remember that due to 09 etc being treated as octal, these values present a problem that could not be fixed with given tool <br/> ";
    }
    // echo "u fail";
   //echo "<br/>";
  }
?>
<ul>
<li>Only numbers and +,-,* and / operators are allowed in the expression. </li>
<li>The evaluation follows the standard operator precedence. </li>
</ul>
</body>
</html>

