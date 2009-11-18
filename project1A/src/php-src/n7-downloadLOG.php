<?php

###################################################
## generate an HTML LOG file for grader to download
## saves Table data for each students including test cases, results, scores, and notes
###################################################

# Uncomment lines and change Content-Type to print file to screen

$fileholder = urldecode($_POST['log_data']);

#print("<p>Passed data: $fileholder </p>");

# Write file to log (deletes previously downloaded file)
$location = "../logs/";
$filename = "GradingFileLOG".date("-M-d-Y-H:m:s").".html";
$LOG = fopen("$location$filename", 'w') or print("Content-type: text/html\n\nThe server can't open $location$filename\n");
fwrite($LOG, "$fileholder\n");
fclose($LOG); 

header("Content-Type:application/x-download");  
header("Content-Disposition:attachment;filename=$filename\n\n");
echo "$fileholder";
exit();
?>
