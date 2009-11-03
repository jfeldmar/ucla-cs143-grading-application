<?php

###################################################
#  This script processes the form submitted in n1_initialize.php
#1. Uploads compressed tar file of submissions to $upload_dir/$tar_filename
#2. Uploads sample PHP calculator solution to $upload_dir/$php_filename
#3. Extract compressed submissions tar file into submissions directory
###################################################

#**********GLOBAL VARIABLES***********
	$safe_filename_characters = "a-zA-Z0-9_.-";
	$upload_dir = "../file-uploads";
	$submissions_dir = "../submissions/";
	$php_filename = "sampleCalculator.php";


	$submission_tarfile_field_name = "student-submissions";
	$sample_solution_field_name = "php-calculator";

	$tar_filename = $_FILES[$submission_tarfile_field_name]['name'];	#"project1A-Submissions.tar"
	$CSS_file = "../html-css/styleSheet.css";
	$BACK_button_link = "n1_initialize.php";

	$form_action_script_n3 = "../php-src/n3_error_process_uploads.php";
#*************************************

#
#Load form parameters
###################################################

if ( !is_uploaded_file($_FILES[$submission_tarfile_field_name]['tmp_name']) )
{
	echo header('Content-type: text/html');
	echo "There was a problem uploading your submissions tar file  (try again or with a smaller file).";	
	exit;
}

#
#HTML
###################################################
?>

<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
<link rel="stylesheet" type="text/css" href="<?php echo $CSS_file; ?>" />

</head>
<body>
<h3 align=left><a class=button style="width:300" href="#" onclick="window.location='<?php echo $BACK_button_link; ?>'"><span>BACK (all files from this run will be deleted)</span></a></h3>

<h1>CS143 - Project 1A Grading Application</h1>
<p align=center><input type=button value='Restart' onClick="location.href='../html-css/start.html'"/></p>

<h2> === Step 2. Processing Uploaded Files... ===</h2>

<?php
#
#Check for correct file extension (*.tar)
###################################################

$tar_path_parts = pathinfo( $_FILES[$submission_tarfile_field_name]['name']);

if (strtolower($tar_path_parts['extension']) != "tar")
{
	 die("Submissions compressed file filename must be a .tar file");
}

#
#Upload .tar file (submissions for Project 1A) into upload directory
###################################################


if (move_uploaded_file($_FILES[$submission_tarfile_field_name]['tmp_name'], "$upload_dir/$tar_filename"))
{
	echo "<p>Opened TAR File...</p>";
	echo "<p>uploading TAR file...</p>";
}
else { die("Cannot open/upload " . $_FILES[$submission_tarfile_field_name]['name']);}


#Print file size and upload status

echo "<p><b>Size of tar file: " . filesize("$upload_dir/$tar_filename") . "</b></p>"; 
echo "<p>uploaded tar file.</p>";

#
#Upload sample PHP Calculator Solution
###################################################

if ( is_uploaded_file($_FILES[$sample_solution_field_name]['tmp_name']))
{
	echo '<p>----------------------------------------------</p>';

	echo "<p>Processing Sample Calculator PHP file upload...</p>";
	
	#
	#Check for correct file extension (*.php)
	#
	$php_parts_info = pathinfo( $_FILES[$sample_solution_field_name]['name']);

	if (strtolower($php_parts_info['extension']) != "php")
	{
		die("Submitted Sample Calculator PHP file filename must be a .php file");
	}

	#
	#Upload .php file (sample calculator solution)
	#

	if (move_uploaded_file($_FILES[$sample_solution_field_name]['tmp_name'], "$upload_dir/$php_filename"))
	{
		echo "<p>Opened PHP File...</p>";
		echo "<p>uploading PHP file...</p>";
	}
	else { die("Cannot open/upload " . $_FILES[$sample_solution_field_name]['name']);}

	#Print file size and upload status

	echo "<p><b>Size of PHP file: " . filesize("$upload_dir/$php_filename") . "</b></p>"; 
	echo "<p>uploaded PHP Sample Calculator Solution file.</p>";
	
	# change mode of file to 644 because after being copied to the file-uploads directory
	# the user changes and needs permission to access file
	chmod("$upload_dir/$php_filename", 0644) or die("unable to change mode of calculator to 644");
	
	#
	# 3. extract into submissions directory
	#
	!system("tar -C $submissions_dir -xf $upload_dir/$tar_filename") or die("Unable to Extract compressed submissions $upload_dir/$tar_filename into $submissions_dir");
}
?>

<!-- Submit/Continue Button -->
<p>
	<a class=button style="width:100; float:right;" href="#" onclick="window.location='<?php echo $form_action_script_n3; ?>'" >
	<span>Continue...</span></a>
</p>
</body>
</html>

