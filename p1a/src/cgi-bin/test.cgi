#!/usr/bin/perl -w

###################################################
## save form data from previous page (i.e. SIDs for submissions to grade) into a log
## load default test cases and allow user to edit them
## functionality: add/remove/save as default test cases
###################################################

use strict;
use CGI;
use CGI::Carp qw ( fatalsToBrowser );
use File::Basename;
use File::Path;

$CGI::POST_MAX = 1024 * 1000 * 10; #max 10 MB
$CGI::DISABLE_UPLOADS = 0;
my $query = new CGI;
my $query_directory = "../test_cases/queries";
my $solutions_directory = "../test_cases/solutions";
my $default_q_dir = "../default-data/test-cases/queries";
my $default_s_dir = "../default-data/test-cases/solutions";
my $sids_to_grade_file = "../logs/SIDstoGrade.csv";

print "Content-type: text/html\n\n";
print <<ENDHTML;
<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />

</head>
<body>

<h1>CS143 - Project 1A Grading Application</h1>
<h2> Confirm/Add/Delete/Save Test Cases </h2>
ENDHTML

# remove current test cases
#mkdir("../mytest/t1/one.txt");
#mkdir("../mytest/t1/two.txt");
#mkdir("../mytest/t2/one.txt");
#mkdir("../mytest/t2/two.txt");
rmtree("../mytest/t2/",0,0);
#rmtree($query_directory, 0,1) or die("Unable to delete contents of $query_directory directory: $!");


print qq(</body></html>);
