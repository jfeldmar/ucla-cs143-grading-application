<html>

<body>
<?php
    echo "Simple Calculator</br>";
?>
</body>


<form action="calculator.php" method="post">
Expression: <input type="text" name="expression" />
<input type="submit" value="Calculate" />
</form> 


<body>
<?php
    // store input into variable
    $exp = $_REQUEST["expression"];
    $inp_str = $exp;
    // do nothing if input has nothing
    if ($exp != "") {
        // set invalid inputs, which are anything
        // other than numbers or +, -, *, /
        $invalid_char = "/[^0-9\+\-\*\/\.]/";
        $valid_order = "/^(([-]?[0-9]+[.]?[0-9]*[\+\-\*\/])*[-]?[0-9]+[.]?[0-9]*)$/";
        // output error result on wrong input patterns
        // checks for invalid character, and input pattern
        if (preg_match($invalid_char, $exp) ||
            !(preg_match($valid_order, $exp))) {
            // output invalid message
            echo $inp_str . " is an invalid expression.";
        }
        else {
            // evaluate the expression
            eval("\$exp = $exp;");
            // output result
            echo "Result</br>" . $inp_str . " = " . $exp . "</br>";
        }
    }
?>
</body>

</html>