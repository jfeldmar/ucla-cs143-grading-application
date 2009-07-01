#!/usr/bin/perl -w

###################################################
#0.!!!!	TEST FOR EXISTENCE OF TEXT::CSV MODULE, if doesn't exist
#		perl -MCPAN -e shell
#		install Text::CSV
#1. This script deletes and recreates directories that 
#   may have files from previous grading runs in them
#   The directory names are:
#	 "../file-uploads", "../submissions", "../temp", "../logs", "../grades", "../test_cases"
#2. Generates HTML Form to accept 
#         compressed submissions file, web server location, and sample solution
# VERBOSE MODE: will list all files and directories as they are deleted
###################################################

use strict;
use File::Path;
use File::Copy;
use CGI;
use CGI::Carp qw ( fatalsToBrowser );
my $query = new CGI;
my $verbose = 0;
my @clear_dirs = ( "../file-uploads", "../submissions", "../temp", "../logs", "../grades", "../test_cases");

print $query->header ( );

print <<ENDHTML;
<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
</head>
<body>
<h1>CS143 - Project 1A Grading Application</h1>
<h2> WELCOME!!!</h2>
ENDHTML

#empty: file-uploads, submissions, temp, logs, grades, test-cases
print "<p>Deleting/recreating directories... </p>";

foreach my $dir (@clear_dirs)
{
	#Delete directory and all contents if it exists
	if (-d $dir){
		if ($verbose) { print "<p>";}
		rmtree($dir,$verbose, 0 ) or die("Cannot remove contents of $dir directory: $!");
		if ($verbose) { print "</p>";}
		if(!$verbose) {
			print 'Deleted '.$dir.', <br />';
		}
	}
	#Recreate empty directory
	##!system("mkdir $dir") or die("Cannot $dir directory: $!");
	mkdir($dir) or die "Cannot create $dir directory: $!";
	if ($verbose){
		print "Created directory $dir";
	}
} 
print "Done.</p>";

#
#Copy saved test cases into the test_cases directory
#####################################################

print "<p>Generating default test-cases...</p>";
!system("tar -C ../test_cases/ -xf ../default-data/test-cases.tar ") or die("Unable to Copy Default Test Cases: $!");;
print "Done.</p>";

#
#HTML Form to upload sumissions compressed file
#(optional)Specify location of webhost directory to display submissions
#(optional)Upload sample solution of Calculator
#####################################################

print <<ENDHTML;

<form method="POST" action="../cgi-bin/n2_process-submissions.cgi" enctype="multipart/form-data" />
<p>Please upload the submission zip file:

	 <input type="file" name="student-submissions" size="50"/>
</p>
<p>Please specify webhost  (ex: /var/www/):

	<input type="text" name="webhost-location" size="50"/>
</p>
<p>(Optional)Please upload sample solution:<br/>
	<p>
	calculator.php:	 <input type="file" name="calculator.php" size="50"/>
	 </p>
</p>

<p align=center><input type="submit" value="Send"/></p>
</form>
</body>
</html>

ENDHTML
