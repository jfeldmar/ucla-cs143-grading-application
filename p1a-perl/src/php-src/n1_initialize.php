<?php

###################################################
#1. This script deletes and recreates directories that 
#   may have files from previous grading runs in them
#   The directory names are:
#	 "../file-uploads", "../submissions", "../temp", "../logs", "../grades", "../test_cases"
#2. Generates HTML Form to accept 
#         compressed submissions file, web server location, and sample solution
# VERBOSE MODE: will list all files and directories as they are deleted
###################################################

$verbose = 0;
$clear_dirs = array( "../file-uploads", "../submissions", "../temp", "../logs", "../grades", "../test_cases");

?>

<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />

</head>
<body>
<h1>CS143 - Project 1A Grading Application</h1>
<h2> WELCOME!!!</h2>


<?php

#empty: file-uploads, submissions, temp, logs, grades, test-cases
echo "<p>Deleting/recreating directories... </p>";

foreach ($clear_dirs as $dir)
{
	#Delete directory and all contents if it exists
	if (file_exists($dir)){
		if ($verbose) { echo "<p>";}
		!system("rm -rf $dir") or die("Cannot remove contents of $dir directory");
		if ($verbose) { echo "</p>";}
		if(!$verbose) {
			echo 'Deleted '.$dir.', <br />';
		}
	}

	#Recreate empty directory
	mkdir($dir) or die("Cannot create $dir directory");
	if ($verbose){
		echo "Created directory $dir";
	}
} 
echo "Done.</p>";

#
#Copy saved test cases from the default-data into the test_cases directory
#####################################################

echo "<p>Generating default test-cases...</p>";
!system("tar -C ../test_cases/ -xf ../default-data/test-cases.tar ") or die("Unable to Copy Default Test Cases");
echo "Done.</p>";

#
#HTML Form to upload sumissions compressed file
#(optional)Specify location of webhost directory to display submissions
#(optional)Upload sample solution of Calculator
#####################################################

?>

<div align=center>
<form name=fileupload method="POST" action="../php-src/n2_process-submissions.php" enctype="multipart/form-data" />
<p>Please upload the submission tar file:<br/>

	 <input type="file" name="student-submissions" size="50"/>
</p>

<!--//<p>Please specify webhost  (ex: /var/www/):
	<input type="text" name="webhost-location" size="50"/>
</p>//-->
<p>(Optional)Please upload sample PHP solution:<br/>
	<p>
	<input type="file" name="php-calculator" size="50"/>
	 </p>
</p>

<p align=center>
<a class=button style="width:200" href="#" onclick="fileupload.submit()"/><span>Send</span></a>
</p>
</form>
</div>
</body>
</html>
