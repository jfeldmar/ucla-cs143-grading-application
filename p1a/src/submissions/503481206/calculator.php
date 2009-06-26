<html>
  <head>
    <title>Barebones Calculator</title>
  </head>
  <body>
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="GET">
      <input type="text" name="expr" />
      <input type="submit" value="Calculate" />
    </form>
    <?php
       function handler($errno, $errstr, $errfile, $errline) {
           throw new Exception($errstr, $errno);
       }

       $expr = $_GET["expr"];

       set_error_handler('handler');

       if (preg_match("/^-?[0-9]*\.?[0-9]+([\+\-\*\/]+-?[0-9]*\.?[0-9]+)*$/", $expr)) {
           try {
               eval("\$ans = $expr;");
               echo "$expr = $ans";
           }
           catch (Exception $e) {
               echo "Error: ", $e->getMessage();
           }
       }
       else {
           echo "Invalid input expression: '$expr'. <br />";
           echo "Only numbers and +,-,* and / operators are allowed; no spaces. <br />";
       }
    ?>
  </body>
</html>
