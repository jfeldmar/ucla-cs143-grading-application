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
my $test_cases_dir = "../test_cases";
my $query_directory = "../test_cases/queries";
my $solutions_directory = "../test_cases/solutions";
my $default_data_tarfile = "../default-data/test-cases2.tar";
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

###################################################
## Run test cases on submissions
## 1. Display save mode and test cases selected
## 2. Store test cases in test_cases directory
##	(if save option is "save", also copy to default-data directory
## 3. Display table summarizing test case output 
##	for selected submissions (use log for reference)
###################################################


## 1. Display save mode and test cases selected
###################################################
my $savetype = $query->param("savetype");
my @formdata = $query->param("tests");
my @queries =();
my @solutions =();

if ($savetype eq "save"){
	print qq(<p>Loading new Test Cases and saving them as Default Test Cases (for future runs)...</p>\n);
}
elsif ($savetype eq "load"){
	print qq(<p>Loading Test Cases...</p>\n);
}else
{
	die("Invalid Test Case save option");
}

print qq(<p align=center>);
print qq(<table border = 1>\n);
print qq(<tr><th>Test Input</th><th>Solution</th></tr>);

#Display test case table
foreach (@formdata){
	my @t = split(/,/, $_);
	
	#save test cases and solutions into 2 different arrays
	push(@queries, @t[0]);
	push(@solutions, @t[1]);
	
	#print test case/solution pairs in table
	print qq(<tr>\n);
	print qq(<td>@t[0]</td>\n);
	print qq(<td>@t[1]</td>\n);
	print qq(</tr>\n);
}

print qq(</table></p>\n);

print qq(<p align=center>To edit Test Cases, click the "Back" button or <a href="javascript:history.back();">click here</a></p>\n);
print qq(<hr/>\n);

## 2. Store test cases in test_cases directory
##	(if save option is "save", also copy to default-data directory
###################################################

# remove current test cases
if (-d $query_directory){
	rmtree($query_directory, 0, 0) or die("Unable to delete contents of $query_directory directory: $!");
}
mkdir($query_directory) or die ("Unable to create $query_directory directory: $!");

if (-d $solutions_directory){
	rmtree($solutions_directory, 0, 0) or die("Unable to delete contents of $solutions_directory directory: $!");
}
mkdir($solutions_directory) or die ("Unable to create $solutions_directory directory: $!");

# create query/solution files for new test cases
foreach my $i (0..$#queries)
{
	my $index = $i + 1;
	open QFILE, ">$query_directory/query$index.txt" or die("Can't open $query_directory/query$#.txt for writing: $!");
	print QFILE "@queries[$i]";
	close QFILE;
	
	open SFILE, ">$solutions_directory/solution$index.txt" or die("Can't open $query_directory/solution$#.txt for writing: $!");
	print SFILE "@solutions[$i]";
	close SFILE;
}

#save to default-data directory if option selected
if ($savetype eq "save"){
	print "<p>Saving as default test-cases...</p>";
	!system("tar  -c -f $default_data_tarfile -C $test_cases_dir queries solutions") or die("Unable to Save Default Test Cases: $!");;
	print "Done.</p>";	
}	
 

## 3. Display table summarizing test case output 
##	for selected submissions (use log for reference)
###################################################

## 3.1 Open log that lists submissions to grade
open FH, $sids_to_grade_file or die("Unable to open $sids_to_grade_file; contains list of submission SID's to grade: $!");

my $sid;
while (chomp($sid = <FH>))
{
	print qq(<p>$sid</p>);
}
close FH;
print qq(</body></html>);
