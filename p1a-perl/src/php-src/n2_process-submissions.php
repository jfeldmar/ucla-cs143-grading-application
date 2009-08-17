<?php

###################################################
#  This script processes the form submitted in n1_initialize.php
#1. Uploads compressed tar file of submissions to $upload_dir/$tar_filename
#2. Uploads sample PHP calculator solution to $upload_dir/$php_filename
#3. Extract compressed tar file into submissions directory
###################################################

$safe_filename_characters = "a-zA-Z0-9_.-";
$upload_dir = "../file-uploads";
$tar_filename = "project1A-Submissions.tar";
$php_filename = "sampleCalculator.php";

#
#Load form parameters
###################################################

#$webhostdir = $_POST('webhost-location');

if ( !is_uploaded_file($_FILES['student-submissions']['tmp_name']) )
{
	echo header('Content-type: text/html');
	echo "There was a problem uploading your submissions tar file (try again or with a smaller file).";
	exit;
}

#
#HTML
###################################################
?>

<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />

</head>
<body>
<h3 align=left><a class=button style="width:300" href="#" onclick="window.location='n1_initialize.php'"><span>BACK (all files from this run will be deleted)</span></a></h3>
<h1>CS143 - Project 1A Grading Application</h1>
<h2> Processing Uploaded Files...</h2>

<?php
#
#Check for correct file extension (*.tar)
###################################################

$tar_path_parts = pathinfo( $_FILES['student-submissions']['name']);

if (strtolower($tar_path_parts['extension']) != "tar")
{
	 die("Submissions compressed file filename must be a .tar file");
}

#
#Upload .tar file (submissions for Project 1A)
###################################################


if (move_uploaded_file($_FILES['student-submissions']['tmp_name'], "$upload_dir/$tar_filename"))
{
	echo "<p>Opened TAR File...</p>";
	echo "<p>uploading TAR file...</p>";
}
else { die("Cannot open/upload " . $_FILES['student-submissions']['name']);}


#Print file size and upload status

echo "<p><b>Size of tar file: " . filesize("$upload_dir/$tar_filename") . "</b></p>"; 
echo "<p>uploaded tar file.</p>";

#
#Upload sample PHP Calculator Solution
###################################################

if ( is_uploaded_file($_FILES['php-calculator']['tmp_name']))
{
	echo '<p>----------------------------------------------</p>';

	echo "<p>Processing Sample Calculator PHP file upload...</p>";
	
	#
	#Check for correct file extension (*.tar)
	#
	$php_parts_info = pathinfo( $_FILES['php-calculator']['name']);

	if (strtolower($php_parts_info['extension']) != "php")
	{
		die("Submitted Sample Calculator PHP file filename must be a .php file");
	}

	#
	#Upload .php file (sample calculator solution)
	#

	if (move_uploaded_file($_FILES['php-calculator']['tmp_name'], "$upload_dir/$php_filename"))
	{
		echo "<p>Opened PHP File...</p>";
		echo "<p>uploading PHP file...</p>";
	}
	else { die("Cannot open/upload " . $_FILES['php-calculator']['name']);}

	#Print file size and upload status

	echo "<p><b>Size of PHP file: " . filesize("$upload_dir/$php_filename") . "</b></p>"; 
	echo "<p>uploaded PHP Sample Calculator Solution file.</p>";
	
	#
	# 3. extract into submissions directory
	#
	!system("tar -C ../submissions/ -xf $upload_dir/$tar_filename") or die("Unable to Extract compressed submissions $upload_dir/$tar_filename into ../submissions/");
}
?>

<p><a class=button style="width:100; float:right;" href="#" onclick="window.location='n3_error_process_uploads.php'" ><span>Continue...</span></a></p>
</body>
</html>

