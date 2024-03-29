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
my $descriptions_directory = "../test_cases/descriptions";
my $default_data_tarfile = "../default-data/test-cases.tar";
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

$" = ', ';
print qq(var unique_sids = new Array(@unique_sids);\n);
$" = ' ';

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

<body onload="load_totals('$default_php');">

<h1>CS143 - Project 1A Grading Application</h1>
<h2> Confirm/Add/Delete/Save Test Cases </h2>

<script type="text/javascript" src="../html-css/js/n5-script-functions.js"></script>
<script type="text/javascript" src="../html-css/js/wz_tooltip.js"></script>

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
my @descriptions =();

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
print qq(<tr><th></th><th>Test Input</th><th>Solution</th><th>Description</th></tr>);

#Display test case table
my $count = 1;
foreach (@formdata){
	my @t = split(/,/, $_);
	
	#save test cases and solutions into 2 different arrays
	push(@queries, @t[0]);
	push(@solutions, @t[1]);
	push(@descriptions, @t[2]);
	
	#print test case/solution pairs in table
	print qq(<tr>\n);
	print qq(<td>$count</td>\n);
	print qq(<td>@t[0]</td>\n);
	print qq(<td>@t[1]</td>\n);
	print qq(<td>@t[2]</td>\n);
	print qq(</tr>\n);
	$count++;
}

print qq(</table></p>\n);

print qq(<p align=center>To edit Test Cases, click the "Back" button or <a href="javascript:history.back();">click here</a></p>\n);

## 2. Store test cases in test_cases directory
##	(if save option is "save", also copy to default-data directory
###################################################

# remove current test cases and their containing directories
if (-d $query_directory){
	rmtree($query_directory, 0, 0) or die("Unable to delete contents of $query_directory directory: $!");
}
mkdir($query_directory) or die ("Unable to create $query_directory directory: $!");

if (-d $solutions_directory){
	rmtree($solutions_directory, 0, 0) or die("Unable to delete contents of $solutions_directory directory: $!");
}
mkdir($solutions_directory) or die ("Unable to create $solutions_directory directory: $!");

if (-d $descriptions_directory){
	rmtree($descriptions_directory, 0, 0) or die("Unable to delete contents of $descriptions_directory directory: $!");
}
mkdir($descriptions_directory) or die ("Unable to create $descriptions_directory directory: $!");

# create query/solution/description files for new test cases
foreach my $i (0..$#queries)
{
	my $index = $i + 1;
	open QFILE, ">$query_directory/query$index.txt" or die("Can't open $query_directory/query$#.txt for writing: $!");
	print QFILE "@queries[$i]";
	close QFILE;
	
	open SFILE, ">$solutions_directory/query$index.txt" or die("Can't open $solutions_directory/query$#.txt for writing: $!");
	print SFILE "@solutions[$i]";
	close SFILE;

	open DFILE, ">$descriptions_directory/query$index.txt" or die("Can't open $descriptions_directory/query$#.txt for writing: $!");
	print DFILE "@descriptions[$i]";
	close DFILE;
}

#save to default-data directory if option selected
if ($savetype eq "save"){
	print "<p align=center>Saving as default test-cases...";
	!system("tar  -c -f $default_data_tarfile -C $test_cases_dir queries solutions descriptions") or die("Unable to Save Default Test Cases: $!");
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

print qq(<form action="n6-downloadCSV.cgi" method="POST" name=getcsv >);
print qq(<input type=hidden id = "csv_data" name="csv_data" value=""/></p>\n);
print qq(<input type=hidden id ="csv_size" name="csv_size" value="4"/></p>\n);
print qq(<div align=center><a class="button" style="width:200" href="javascript:javascript:submit_csv(this);"><span>DOWNLOAD CSV</span></a></div>\n);
print qq(</form>);


print qq(<div align=center><a class="button"  style="width:75" href="javascript:show_all('<span>Show</span>');"/><span>Show All</span></a>\n);
print qq(<a class="button"  style="width:75" href="javascript:hide_all('<span>Hide</span>');"/><span>Hide All</span></a></div>\n);
print qq(<p align=center>*****************************************************************</p>);

foreach my $sid (@unique_sids)
{
       ## separator for each submission's code
       print qq(\n\n\n<!-- ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -->\n);
       print qq(<!-- ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^sid:$sid^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -->\n);
       print qq(<!-- ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -->\n\n\n);

       print qq(<p>);
       print qq(<a class="button ShowHideButton" name="$sid" href="#"  style="width:60" onclick="javascript:ShowHideSection('$sid', this);"><span>Show</span></a>\n);
#       print qq(<br/><input type="button" class="$sid" style="width:4em" onclick="javascript:ShowHideSection('$sid', this);" value='Show'>\n);

       print qq(&nbsp;&nbsp;Test Case Results for Student $sid\n);
       print qq(<a class=button href="#"  style="width:110" onclick="javascript:update_total_score('$sid');" ><span>Sum Scores</span></a>\n);
#       print qq(<a class=button href="#"  style="width:105" onclick="javascript:update_notes('$sid');" ><span>Concat Notes</span></a></p>\n);

#       print qq(<input type="button" onclick="javascript:update_total_score('$sid');" value="Sum Scores" >\n);
#       print qq(<input type="button" onclick="javascript:update_notes('$sid');" value="Concat Notes" >\n);

       print qq(<div class="submissions" id="$sid" style="overflow:hidden;display:none">\n);

       print qq(<p align=center>Graded File: <a class="link_editable" href="$submissions_directory/$sid/$default_php" target=_blank >$default_php</a>\n);
       print qq(&nbsp;or&nbsp;<a class=button style="width:200" href="#" onclick="ChooseFilePopUp('$pop_up_window','$sid');"/><span>Choose PHP File to Grade</span></a></p>\n);
       
       print qq(<table width=90% border="1" align="center">);
       print qq(<tr><th></th><th>QUERY</th><th>Sample Solution</th><th>RESULT</th><th>Score</th><th>Notes</th></tr>\n);
       
       my $escape_str = "";
       # For each Query
       ###################################################
       foreach my $i (0..$#queries)
       {
       		my $temp = $i + 1;
	       print qq(<tr>);
	       print qq(<td>$temp</td>\n);

	       # link to student's solution with given input
	       print qq(<td>\n);
	       $escape_str =  CGI::escape("@queries[$i]");
	       print qq(<a class="php_editable" href=\"$submissions_directory/$sid/$default_php?expr=$escape_str" target=_blank > @queries[$i] </a>\n);
	       print qq(</td>\n);

	       # expected result (link to sample solution)
	       print qq(<td>\n);
	       $escape_str = CGI::escape("@queries[$i]");
	       print qq(<a class="sampleresult" href="$sample_php?expr=$escape_str" target=_blank > @solutions[$i] </a>\n);
	       print qq(</td>\n);
	       
	       # extract student's result for given query
	       $escape_str = CGI::escape("@queries[$i]");
	       print qq(<td class="phpresult" id="$submissions_directory/$sid/$default_php?expr=$escape_str">\n);
	       print qq(<script type="text/javascript">\n);
	       print qq( document.write(get_result("$submissions_directory/$sid/$default_php?expr=$escape_str"));\n);
	       print qq(</script>\n);
	       print qq(</td>\n);

	       # score based on matching solution and output
	       print qq(<td class="qscore" onClick="editCell(this, 'number', '$sid');">0</td>\n);

	       # comments based on score
#	       print qq(<td class="qnotes" onClick="editCell(this, 'text', '$sid');">@descriptions[$i]</td>\n);
	       print qq(<td class="qnotes">@descriptions[$i]</td>\n);
	       print qq(</tr>\n);
       }
       ###################################################
       
       print qq(<span><tr>\n);
       print qq(<td colspan=4 style="font-weight:bold" >Total Score</td>\n);
       print qq(<td class="tscore" style="font-weight:bold" ></td>\n);
       print qq(<td class="num_correct_score" style="font-weight:bold" ></td>\n);
       print qq(</tr>\n);
       print qq(<tr><td colspan=6 class="tnotes" onClick="editCell(this, 'text', '$sid');">Notes:</td></tr></span>\n);

       print qq(</table>\n);
       print qq(</div>\n);
}
print qq(<p align=center>*****************************************************************</p>);

print qq(<div align=center><a class="button"  style="width:75" href="javascript:show_all('<span>Show</span>');"/><span>Show All</span></a>\n);
print qq(<a class="button"  style="width:75" href="javascript:hide_all('<span>Hide</span>');"/><span>Hide All</span></a></div>\n);

print qq(</body></html>);
