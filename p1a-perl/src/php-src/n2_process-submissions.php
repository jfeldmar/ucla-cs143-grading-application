<?php

###################################################
#  This script processes the form submitted in n1_initialize.cgi
#1. Uploads compressed tar file of submissions to $upload_dir/$zip_filename
#2. Uploads sample PHP calculator solution to $upload_dir/$php_filename
#3. Extract compressed tar file into submissions directory
###################################################


$safe_filename_characters = "a-zA-Z0-9_.-";
$upload_dir = "../file-uploads";
$zip_filename = "project1A-Submissions.tar";
$php_filename = "sampleCalculator.php";

?>

<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />

</head>
<body>
<h3 align=left><a class=button style="width:300" href="#" onclick="window.location='n1_initialize.cgi'"><span>BACK (all files from this run will be deleted)</span></a></h3>
<h1>CS143 - Project 1A Grading Application</h1>
<h2> Processing Uploaded Files...</h2>


<?php
#
#Load form parameters
###################################################

$zipfile = $_POST["student-submissions"] or die("No File Selected");
$webhostdir = $_POST["webhost-location"];
$sampleCalcFile = $_POST["calculator.php"];

if ( !$zipfile )
{
 echo "There was a problem uploading your submissions zip file (try a smaller file).";
 exit();
}


#
#Check for correct file extension (*.tar)
###################################################

my ( $zip_name, $zip_path, $zip_extension ) = fileparse ( $zipfile, '\..*' );

if (lc($zip_extension) ne ".tar")
{
	 die "Submissions compressed file filename must be a .tar file";
}

#
#Upload .tar file (submissions for Project 1A)
###################################################

my $upload_filehandle = $query->upload("student-submissions") or die "Cannot open $zipfile: $!";


open ( UPLOADFILE, ">$upload_dir/$zip_filename" ) or die "$!";
binmode UPLOADFILE;

echo "<p>Opened Zip File...</p>";

echo "<p>uploading zip file...</p>";
while ( <$upload_filehandle> )
{
	echo UPLOADFILE;
} 

close UPLOADFILE; 

#
#Print file size and upload status
###################################################

my $filesize = -s "$upload_dir/$zip_filename";
echo "<p><b>Size of zipfile: " . $filesize . "</b></p>"; 
echo "<p>uploaded zip file.</p>";

#
#Upload sample PHP Calculator Solution
###################################################

if ( $sampleCalcFile )
{
	echo "<p>Processing Sample Calculator PHP file upload...</p>";
	
	#
	#Check for correct file extension (*.tar)
	#
	my ( $php_name, $php_path, $php_extension ) = fileparse ( $sampleCalcFile, '\..*' );

	if ($php_extension ne ".php")
	{
	 die "Submitted Sample Calculator PHP file filename must be a .php file";
	}

	#
	#Upload .php file (sample calculator solution)
	#

	$upload_filehandle = $query->upload("calculator.php") or die "Cannot open $zipfile: $!";


	open ( UPLOADFILE, "+>$upload_dir/$php_filename" ) or die "$!";
	binmode UPLOADFILE;

	echo "<p>Opened PHP File...</p>";

	echo "<p>uploading PHP file...</p>";
	while ( <$upload_filehandle> )
	{
		echo UPLOADFILE;
	} 

	close UPLOADFILE; 

	#
	#Print file size and upload status
	#

	my $filesize = -s "$upload_dir/$php_filename";
	echo "<p><b>Size of PHP file: " . $filesize . "</b></p>"; 
	echo "<p>uploaded PHP Sample Calculator Solution file.</p>";
	
	#
	# 3. extract into submissions directory
	#
	!system("tar -C ../submissions/ -xf $upload_dir/$zip_filename") or die("Unable to Extract compressed submissions $upload_dir/$zip_filename into ../submissions/ : $!");


}

echo '<p><a class=button style="width:100; float:right;" href="#" onclick="window.location=\'n3_error_process_uploads.cgi\'" ><span>Continue...</span></a></p>';
echo "</html>";

?>
