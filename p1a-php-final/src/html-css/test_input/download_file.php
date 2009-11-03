<?php

$filename = $_GET['file'];
#$filename = './index-ORIGINAL.php';

$LOG = fopen($filename, 'r') or print("Content-type: text/html\n\nThe server can't open $filename\n");
$data = fread($LOG, filesize($filename));
fclose($LOG); 

header("Content-Type:application/x-download");  
header("Content-Disposition:attachment;filename='".basename($filename)."'\n\n");
echo "$data";
exit();

?>
