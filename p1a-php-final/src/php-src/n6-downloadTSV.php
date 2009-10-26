<?php

###################################################
## generate a CSV (comma separated value) file for user to download
## saves SID, total score, fraction correct, and total notes
###################################################

# Uncomment lines and change Content-Type to print file to screen

$fileholder = urldecode($_POST['csv_data']);

#print("<p>Passed data: $fileholder </p>");


# Write file to log (deletes previously downloaded file)
$location = "../logs/";
$filename = "SavedGradingFile".date("-M-d-Y-H:m:s").".tab";
$LOG = fopen("$location$filename", 'w') or print("Content-type: text/html\n\nThe server can't open $location$filename\n");
fwrite($LOG, "$fileholder\n");
fclose($LOG); 

header("Content-Type:application/x-download");  
header("Content-Disposition:attachment;filename=$filename\n\n");
echo "$fileholder";
exit();
?>
