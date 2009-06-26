<html>

  <head>
    <title>Calculator</title>
  </head>

  <body>

    <h1>Calculator</h1>

    <form action="calculator.php" method="GET">
    <input type="text" name="input">
    <input type="submit" value="Calculate">
    <br />

    <?php

      if($_GET["input"])
      {
        $equ = $_GET["input"];

	if(preg_match("/[a-z]/",$equ))
	  echo "Invalid input.";

	else if(preg_match("/[A-Z]/",$equ))
	  echo "Invalid input.";
	
	else
	{
          eval("\$ans = $equ ;");
	  echo "Answer = ".$ans;
	}
      }

    ?>
 
  </body>

</html>