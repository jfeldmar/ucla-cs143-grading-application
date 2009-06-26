
<html>
<title>KIM'S CALCULATOR</title>

<font size="6">
<?php
echo "Calculator <br/>";
?>

<font size="3">
<?php
echo "Type an expression in the following box (e.g., 10.5+20*3/25).<br/><br/>";
?>

<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="post">
<input type="text" name="equation"/>
<input type="submit" value="Calculate" name='sub'/>    
</form>


<?php if(isset($_POST['sub'])){ ?>
    <font size="6">
    <?php echo "<br/><br/>Results<br/>"; ?>
    <font size="3">
<?php
    $inputeq = $_POST['equation'];
    $pattern = '/^-?[0-9]*\.?[0-9]*([+*\/-]{1}-?[0-9]*\.?[0-9]*|[0-9]*)*$/';
   
    $div0pat = '/^.*\/0.*$/';
	
	if(preg_match($div0pat, $inputeq) == 1)
		echo "Invalid  input expression: " . $inputeq;
   	elseif($inputeq[0]=='/' || $inputeq[0]=='*')
		echo "Invalid input expression: " . $inputeq;
    elseif(preg_match($pattern, $inputeq) == 1)
        {
			eval("\$num = $inputeq;");
			echo $inputeq . " = " . $num;
        }
    else
        echo "Invalid input expression: " . $inputeq;
}
?>

</html>