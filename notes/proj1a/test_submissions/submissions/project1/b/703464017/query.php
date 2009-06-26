<html>
    <head>
        <title>Project 1B: SQL Queries</title>
    </head>

    <body>
        <form action="<?=$_SERVER['PHP_SELF']?>" method="post">
            <p>
                Student Name: Ho Ching Lam<br>
                Student ID: 703464017
            </p>
            <h4>
                Type an SQL query in the following box:
            </h4>
            <textarea name="textarea" rows="9" cols="60"></textarea>
            <input TYPE="SUBMIT" VALUE="Submit">
        </form>
        <?php
            // Errors handling
            function errhandler($errmsg) {
                print "Error: $errmsg <br>";
                exit(1);
            }
            $query = $_POST["textarea"];

            // Establishing a Connection
            $db_connection = mysql_connect("localhost", "cs143", "");
            if ($db_connection) { // Connection succeeded

                // Selecting a Database
                if (mysql_select_db("CS143", $db_connection)) { // Select Success
                    print '<h3>Results from MySQL:</h3>';
                    // Issuing Queries
                    $result = mysql_query($query, $db_connection);
                    if ($result) { // Query succeeded
                        $query_type = substr($query, 0, strpos($query, ' '));
                        if ((strcasecmp($query_type, 'DELETE') != 0)
                            && (strcasecmp($query_type, 'INSERT') != 0)
                            && (strcasecmp($query_type, 'REPLACE') != 0)
                            && (strcasecmp($query_type, 'UPDATE') != 0)) {
                                // Retrieving Result by using table
                                print '<table border="1">';
                                print '<thead><tr>';

                                while ($field = mysql_fetch_field($result))
                                    print "<th>$field->name</th>";

                                print '</tr></thead>';
                                while ($row = mysql_fetch_row($result)) {
                                    print '<tr>';
                                    for ($i = 0; $i < sizeof($row); $i++)
                                        print "<td>$row[$i]</td>";
                                    print '</tr>';
                                }
                                print '</table>';

                                mysql_close($db_connection);
                            }
                            else
                                printf ("%s %d rows", strtoupper($query_type),
                                        mysql_affected_rows());
                    }
                    else // Query failed
                        errhandler("Bad query");
                }
                else // Select failed
                    errhandler("Database selection failed");
            }
            else // Connection failed
                errhandler(mysql_error($db_connection));

        ?>
    </body>
</html>