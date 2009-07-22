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

my $default_php = "calculator.php";
my $sample_php = "../file-uploads/sampleCalculator.php";
my $submissions_directory = "../submissions";
my $temp_directory = "../temp";
my $pop_up_window = "../html-css/choosefile-popup.html";

print qq(Content-type: text/html\n\n);
print qq(<html><head>\n);
print qq(<title>CS143 - Project 1A Grading Application</title>\n);
print qq(<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />\n);
print qq(<script type="text/javascript">\n);

## for each submission, print a javascript array containing all files submitted by that student
###################################################

## Open log that lists submissions to grade, create array of unique SIDs
open FH, $sids_to_grade_file or die("Unable to open $sids_to_grade_file; contains list of submission SID's to grade: $!");

my @ids = ();
while (chomp(my $sid = <FH>))
{
	push(@ids, $sid);
}
close FH;

my %hash = map { $_, 1 } @ids;
my @unique_sids = keys %hash;

# for each unique sid, create "files-sid" array listing all files submitted
foreach my $sid (@unique_sids)
{
	# open directory and read files,
	# then print javascript array variable containing the data
	my $declaration = "";

	opendir DIR, "$submissions_directory/$sid" or die("Can't open submission directory $submissions_directory/$sid for $sid: $!");
	my @files = grep { $_ ne '.' && $_ ne '..' } readdir DIR;

	# print files to array
	$declaration = "var files_$sid = new Array(";
	
	my $i = 0;
	foreach my $submitted_file (@files){
		unless (-d "$submissions_directory/$sid/$submitted_file"){
			#if ($submitted_file =~ m/.php/){
			if ($i > 0){
				$declaration .= ", ";
			}
			$declaration .= qq("$submitted_file");
			$i++;
		}
	}
	$declaration .= ")";
	closedir DIR;
	
	print "$declaration\n";
}

print <<ENDHTML;
</script>
</head>
<body onload="update_totals();">

<h1>CS143 - Project 1A Grading Application</h1>
<h2> Confirm/Add/Delete/Save Test Cases </h2>

<script type="text/javascript" src="../html-css/js/n5-script-functions.js"></script>

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
print qq(<tr><th></th><th>Test Input</th><th>Solution</th></tr>);

#Display test case table
my $count = 1;
foreach (@formdata){
	my @t = split(/,/, $_);
	
	#save test cases and solutions into 2 different arrays
	push(@queries, @t[0]);
	push(@solutions, @t[1]);
	
	#print test case/solution pairs in table
	print qq(<tr>\n);
	print qq(<td>$count</td>\n);
	print qq(<td>@t[0]</td>\n);
	print qq(<td>@t[1]</td>\n);
	print qq(</tr>\n);
	$count++;
}

print qq(</table></p>\n);

print qq(<p align=center>To edit Test Cases, click the "Back" button or <a href="javascript:history.back();">click here</a></p>\n);

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
	print "<p align=center>Saving as default test-cases...";
	!system("tar  -c -f $default_data_tarfile -C $test_cases_dir queries solutions") or die("Unable to Save Default Test Cases: $!");
	print "Done.</p>";
}	
print qq(<hr/>\n);
print qq(<p align=center><a href="$sample_php" target=_blank>Your Sample Submission</a> &nbsp;&nbsp;&nbsp;\n);
print qq((<a href="$sample_php?expr=2%2B3" target=_blank>Simple Test Case: 2+3</a>)</p>\n);
print qq(<hr/>\n);
## 3. Display table summarizing test case output 
##	for selected submissions (use log for reference)
###################################################

## 3. Display table summarizing test case output 
##	for selected submissions (use log for reference)

foreach my $sid (@unique_sids)
{
       print qq(<br/><input type="button" style="width:4em" onclick="javascript:ShowHideSection('$sid', this);" value="Show" >\n);
       print qq(&nbsp;&nbsp;Test Case Results for Student $sid\n);
       print qq(<input type="button" onclick="javascript:update_total_score('$sid');" value="Sum Scores" >\n);
       print qq(<input type="button" onclick="javascript:update_notes('$sid');" value="Concat Notes" >\n);
       
       print qq(<div class="submissions" id=$sid style="overflow:hidden;display:none">\n);
       print qq(<p align=center>Graded File: <a class="link_editable" href="$submissions_directory/$sid/$default_php" target=_blank >$default_php</a>\n);
       print qq(&nbsp;or&nbsp;<input type="button" value="Choose PHP File to Grade" onclick="ChooseFilePopUp('$pop_up_window','$sid');"/></p>\n);
       
       print qq(<table width=90% border="1" align="center">);
       print qq(<tr><th></th><th>QUERY</th><th>Sample Solution</th><th>RESULT</th><th>Score</th><th>Notes</th></tr>);
       
       # For each Query
       ###################################################
       foreach my $i (0..$#queries)
       {
       		my $temp = $i + 1;
	       print qq(<tr>);
	       print qq(<td>$temp</td>);

	       # link to student's solution with given input
	       print qq(<td>);
	       print qq(<script type="text/javascript">);
	       print qq(document.write('<a class="php_editable" href=\"$submissions_directory/$sid/$default_php?expr=' + encodeURIComponent("@queries[$i]") + '\" target=_blank > @queries[$i] </a>'););
	       print qq(</script>);
	       print qq(</td>);

	       # expected result (link to sample solution)
	       print qq(<td>);
	       print qq(<script type="text/javascript">);
	       print qq(document.write('<a href=\"$sample_php?expr=' + encodeURIComponent("@queries[$i]") + '\" target=_blank > @solutions[$i] </a>'););
	       print qq(</script>);
	       print qq(</td>);
	       
	       # extract student's result for given query
	       print qq(<td>);
	       print qq(<script type="text/javascript">);
	       print qq(document.write('<input type=button id=\"$submissions_directory/$sid/$default_php?expr=' + encodeURIComponent("@queries[$i]") + '\" onload="get_result(this);" value="Plug In Result">'););
	       print qq(</script>);
	       print qq(</td>);

	       # score based on matching solution and output
	       $temp = 100 + $temp;
	       print qq(<td class="qscore" onClick="editCell(this, 'number', $sid);">$temp</td>);

	       # comments based on score
	       print qq(<td class="qnotes" onClick="editCell(this, 'text', $sid);">Correct?</td>);
	       print qq(</tr>);
       }
       ###################################################
       
       print qq(<tr>);
       print qq(<td colspan=4>Total Score</td>);
       print qq(<td class="tscore">($#queries + 1) * 100</td>);
       print qq(<td class="tnotes">Notes:</td>);
       print qq(</tr>);

       print qq(</table>);
       print qq(</div>);
}

print qq(</body></html>);
