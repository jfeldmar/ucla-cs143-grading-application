<HTML>
    <HEAD>
        <?php $title = "Calculator" ?>
        <TITLE><?php print "$title"; ?></TITLE>
    </HEAD>

    <BODY bgcolor=white>
        <H1><?php print "$title"; ?></H1>

        <FORM action="<?=$_SERVER['PHP_SELF']?>" method="post">
            <INPUT TYPE="text" NAME="formula" VALUE="" SIZE="" MAXLENGTH="">
            <INPUT TYPE="submit" VALUE="Calculate">
        </FORM>
        <?php
            $param = $_POST["formula"];
            if(preg_match("/^[0-9]+([.][0-9])?([\+\-\*\/][0-9]+([.][0-9])?)*$/",$param, $matches)) {
                eval("\$ans = $param;");
                print "$param = ".$ans;
            }
            else
              print "Invalid input expression ".$param;
        ?>
    </BODY>
</HTML>