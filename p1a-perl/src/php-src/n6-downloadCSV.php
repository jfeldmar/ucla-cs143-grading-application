<?php

###################################################
## generate a CSV (comma separated value) file for user to download
## saves SID, total score, and total notes
###################################################

# Uncomment lines and change Content-Type to print file to screen

$file = urldecode($_POST['csv_data']);
$size = urldecode($_POST['csv_size']);
$fileholder = "";
$fields = explode(",", $file);


#print("<p>Passed array width: $size </p>");
#print("<p>Passed data: $file </p>");


# Uncomment lines and change Content-Type to print file to screen
#print("<pre>");
for ($i = 0; $i < count($fields); $i++)
{
	for ($j = 0; $j < $size; $j++)
	{
//		$fields[$i] =~ s/"/""/g;
		$fileholder .= "$fields[$i],";
#		echo "$fields[$i], ";
		$i++;
	}
	$i--;
	$fileholder .= "\n";
#	echo "\n\r";
}
#print("</pre>");

# Write file to log (deletes previously downloaded file)
$LOG = fopen("../logs/SavedGradingFile".date("-M-d-Y-H:m:s").".csv", 'w') or print("Content-type: text/html\n\nThe server can't open ../logs/SavedGradingFile.csv\n");
fwrite($LOG, "$fileholder\n");
fclose($LOG); 

header("Content-Type:application/x-download");  
header("Content-Disposition:attachment;filename=GradesCSV".date("-M-d-Y-H:m:s").".csv\n\n");
echo "$fileholder";
exit();
?>
