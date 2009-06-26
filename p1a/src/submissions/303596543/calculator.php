<!-- 
    Document   : calcpage
    Created on : Oct 7, 2008, 2:15:53 PM
    Author     : cody
-->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
  <head>
      <title>Calculator - Project 1A</title>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
  </head>
  <body>
  <big style="font-size:22pt; color:aqua">Calculator</big><br><br>
      Type an expression in the following box (e.g., 10.5+20*3/25): <br>
   <form  method="GET">
    Equation: <input type="text" name="calcstr" size="20"/>
    <!-- Age: <input type="text" name="age" /-->
    <input type="submit" value="Calculate" />
   </form>
    Only numbers and +,-,* and / operators are allowed in the expression.
   <br><br>
   <big style="font-size:22pt; color:aqua">Result</big><br>
    <?php
    if($_GET["calcstr"])
    {
	    $calcin = $_GET["calcstr"];
	    //echo "Welcome ".$calcin."<br/>";
	    //<!--preg_match("/[^\.\/]+\.[^\.\/]+$/","http://www.php.net",$matches);-->
	    $retval = preg_match("/[A-Za-z\!\@\#\$\%\^\&\(\)\<\>\?\{\}\,\;\:\~\=\"\`\|]/",
	        $calcin,$matches);
		//echo "matches: {$matches[0]} {$matches[1]} {$matches[2]} <br>";
		
		if ($retval > 0)
	    {	// match found therefore invalid
	        echo "Invalid expression:  {$calcin} --> {$matches[0]}\n";
	    }
	    else
	    {
			//$matches = $calcin;
	        //echo "Calculator expression: ".$calcin."<br>";
	        eval("\$ans = $calcin ;");
	        //echo "ans= ".$ans."<br>";
			echo "Expression: ".$calcin." = ".$ans."<br>";
	    }
	}
    ?>
  </body>
</html>