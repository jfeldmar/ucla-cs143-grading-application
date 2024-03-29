<?php

###################################################
## save form data from previous page (i.e. SIDs for submissions to grade) into a log
## load default test cases and allow user to edit them
## functionality: add/remove/save as default test cases
###################################################

#**********GLOBAL VARIABLES***********
	$CSS_file = "../html-css/styleSheet.css";

	$test_cases_dir = "../test_cases";
	$query_directory = "../test_cases/queries";
	$solutions_directory = "../test_cases/solutions";
	$descriptions_directory = "../test_cases/descriptions";
	$default_data_tarfile = "../default-data/test-cases.tar";
	$sids_to_grade_file = "../logs/SIDstoGrade.csv";

	$default_php = "calculator.php";
	$sample_php = "../file-uploads/sampleCalculator.php";
	$submissions_directory = "../submissions";
	$temp_directory = "../temp";
	$pop_up_window = "../html-css/choosefile-popup.html";
	
	$download_csv_file_link = "n6-downloadCSV.php";
	$csv_values_per_submission = "4";
#*************************************

echo header('Content-type: text/html');
?>

<html><head>
<title>CS143 - Project 1A Grading Application</title>
	<link rel="stylesheet" type="text/css" href="<?php echo $CSS_file; ?>" />

<script type="text/javascript">

<?php

## for each submission, print a javascript array containing all files submitted by that student
###################################################

## Open log that lists submissions to grade, create array of unique SIDs
###################################################
$FH = fopen($sids_to_grade_file, 'r') or die("Unable to open $sids_to_grade_file; contains list of submission SID's to grade");
$temp_array = explode( "\n", fread($FH, filesize("$sids_to_grade_file")));
fclose($FH);
if ( $temp_array == 'FALSE' or count($temp_array) == 0)
	die("Unable to extract submitted SID's from submissions.csv file");
$unique_sids = array_unique($temp_array);

$last = array_pop($unique_sids);
echo "var unique_sids = new Array(" . implode( ",", $unique_sids) . $last . ");\n";
###################################################

# for each unique sid, create "files-sid" array listing all files submitted
foreach ($unique_sids as $sid)
{
	# open directory and read files,
	# then print javascript array variable containing the data
	$declaration = "";

	$files = array();
	$DIR = opendir("$submissions_directory/$sid") or die("Can't open submission directory $submissions_directory/$sid for $sid");
	while ( $f = readdir($DIR))
	{
    	    if ($f != "." && $f != "..")
		    array_push($files, $f);
	}
	closedir($DIR);

	# print files to array
	$declaration = "var files_$sid = new Array(";
	
	$i = 0;
	foreach ($files as $submitted_file){
		if (file_exists("$submissions_directory/$sid/$submitted_file")){
			#if ($submitted_file =~ m/.php/){
			if ($i > 0){
				$declaration .= ", ";
			}
			$declaration .= "\"$submitted_file\"";
			$i++;
		}
	}
	$declaration .= ");";
	
	print "$declaration\n";
}
?>

</script>
</head>

<?php echo "<body onload=\"load_totals('".$default_php."');\">"; ?>

<h1>CS143 - Project 1A Grading Application</h1>
<h2> Confirm/Add/Delete/Save Test Cases </h2>

<script type="text/javascript" src="../html-css/js/n5-script-functions.js"></script>
<script type="text/javascript" src="../html-css/js/wz_tooltip.js"></script>

<?php

###################################################
## Run test cases on submissions
## 1. Display save mode and test cases selected
## 2. Store test cases in test_cases directory
##	(if save option is "save", also copy to default-data directory
## 3. Display table summarizing test case output 
##	for selected submissions
###################################################


## 1. Display save mode and test cases selected
###################################################
if ( !isset($_POST['savetype'])){
	die('Unable to extract savetype variable from previous form');
}
if ( !isset($_POST['tests'])){
	die('Unable to extract test cases from previous form');
}

# store test cases in local array
$formdata = array();
foreach ( $_POST['tests'] as $tests )
{
	array_push($formdata, $tests);
}

$savetype = $_POST['savetype'];
if (!strcmp($savetype, "save")){
	echo "<p>Loading new Test Cases and saving them as Default Test Cases (for future runs)...</p>\n";
}
else if (!strcmp($savetype,"load")){
	echo "<p>Loading Test Cases...</p>\n";
}else{
	die("Invalid Test Case save option");
}
?>

<p align=center>
<table border = 1>
<tr><th></th><th>Test Input</th><th>Solution</th><th>Description</th></tr>

<?php
#Display test case table
$count = 1;
$queries =array();
$solutions =array();
$descriptions =array();

foreach ($formdata as $d){
	$t = explode(",", $d);
	
	#save test cases and solutions into 2 different arrays
	array_push($queries, $t[0]);
	array_push($solutions, $t[1]);
	array_push($descriptions, $t[2]);
	
	#print test case/solution pairs in table
	echo "<tr>\n";
	echo "<td>$count</td>\n";
	echo "<td>$t[0]</td>\n";
	echo "<td>$t[1]</td>\n";
	echo "<td>$t[2]</td>\n";
	echo "</tr>\n";
	$count++;
}

echo "</table></p>\n";

echo "<p align=center>To edit Test Cases, click the \"Back\" button or <a href=\"javascript:history.back();\">click here</a></p>\n";

## 2. Store test cases in test_cases directory
##	(if save option is "save", also copy to default-data directory
###################################################

# remove current test cases and their containing directories
if (file_exists($query_directory)){
	!system("rm -rf $query_directory") or die("Unable to delete contents of $query_directory directory");
}
mkdir($query_directory) or die ("Unable to create $query_directory directory");

if (file_exists($solutions_directory)){
	!system("rm -rf $solutions_directory") or die("Unable to delete contents of $solutions_directory directory");
}
mkdir($solutions_directory) or die ("Unable to create $solutions_directory directory");

if (file_exists($descriptions_directory)){
	!system("rm -rf $descriptions_directory") or die("Unable to delete contents of $descriptions_directory directory");
	}
mkdir($descriptions_directory) or die ("Unable to create $descriptions_directory directory");

# create query/solution/description files for new test cases
for ($i = 0; $i < count(queries); $i++)
{
	$index = $i + 1;
	$QFILE = fopen("$query_directory/query$index.txt", 'w+') or die("Can't open $query_directory/query$#.txt for writing");
	fwrite( $QFILE, "$queries[$i]");
	fclose($QFILE);
	
	$SFILE = fopen("$solutions_directory/query$index.txt", 'w+') or die("Can't open $solutions_directory/query$#.txt for writing");
	fwrite( $SFILE, "$solutions[$i]");
	fclose($SFILE);

	$DFILE = fopen("$descriptions_directory/query$index.txt", 'w+') or die("Can't open $descriptions_directory/query$#.txt for writing");
	fwrite( $DFILE, "$descriptions[$i]");
	fclose($DFILE);
}

#save to default-data directory if option selected
if (!strcmp($savetype,"save")){
	echo "<p align=center>Saving as default test-cases...";
	!system("tar  -c -f $default_data_tarfile -C $test_cases_dir queries solutions descriptions") or die("Unable to Save Default Test Cases");
	echo "Done.</p>";
}	
echo "<hr/>\n";
echo "<p align=center><a href=\"$sample_php\" target=_blank>Your Sample Submission</a> &nbsp;&nbsp;&nbsp;\n";
echo "(<a href=\"$sample_php?expr=2%2B3\" target=_blank>Simple Test Case: 2+3</a>)</p>\n";
echo "<hr/>\n";

## 3. Display table summarizing test case output 
##	for selected submissions (use log for reference)
###################################################

?>

<!-- Link/Form to download CSV containing final grades -->
<form action="<?php echo $download_csv_file_link ?>" method="POST" name=getcsv >
<input type=hidden id = "csv_data" name="csv_data" value=""/></p>
<input type=hidden id ="csv_size" name="csv_size" value="<?php echo $csv_values_per_submission;?>"/></p>
<div align=center><a class="button" style="width:200" href="javascript:javascript:submit_csv(this);"><span>DOWNLOAD CSV</span></a></div>
</form>

<!-- Table Containing Submissions Data -->
<div align=center>

<!-- Show All / Hide All Buttons -->
<a class="button"  style="width:75" href="javascript:show_all('<span>Show</span>');"/><span>Show All</span></a>
<a class="button"  style="width:75" href="javascript:hide_all('<span>Hide</span>');"/><span>Hide All</span></a></div>

<p align=center>*****************************************************************</p>

<?php

#Display Data for each Submission (sid directory)
###################################################
foreach ($unique_sids as $sid)
{
       ## separator for each submission's code
       echo "\n\n\n<!-- ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -->\n";
       echo "<!-- ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^sid:$sid^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -->\n";
       echo "<!-- ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -->\n\n\n";

	## Show/Hide Button
       echo "<p>";
       echo "<a class=\"button ShowHideButton\" name=\"$sid\" href=\"#\"  style=\"width:60\" onclick=\"javascript:ShowHideSection('$sid', this);\"><span>Show</span></a>\n";
#       echo "<br/><input type=\"button\" class=\"$sid\" style=\"width:4em\" onclick=\"javascript:ShowHideSection('$sid', this);\" value='Show'>\n";

	## "Sum Score" Button (for current student only)
       echo "&nbsp;&nbsp;Test Case Results for Student $sid\n";
       echo "<a class=button href=\"#\"  style=\"width:110\" onclick=\"javascript:update_total_score('$sid');\" ><span>Sum Scores</span></a>\n";
#       echo "<a class=button href=\"#\"  style=\"width:105\" onclick=\"javascript:update_notes('$sid');\" ><span>Concat Notes</span></a></p>\n";

#       echo "<input type=\"button\" onclick=\"javascript:update_total_score('$sid');\" value=\"Sum Scores\" >\n";
#       echo "<input type=\"button\" onclick=\"javascript:update_notes('$sid');\" value=\"Concat Notes\" >\n";

       echo "<div class=\"submissions\" id=\"$sid\" style=\"overflow:hidden;display:none\">\n";

	## Name of/ Link to file being graded now
       echo "<p align=center>Graded File: <a class=\"link_editable\" href=$submissions_directory/$sid/$default_php target=_blank > $default_php </a>\n";
        ## Button to change file being graded
       echo "&nbsp;or&nbsp;<a class=button style=\"width:200\" href=\"#\" onclick=\"ChooseFilePopUp('$pop_up_window','$sid');\"/><span>Choose PHP File to Grade</span></a></p>\n";

	## Query Table (Student's solution, Sample Solution, Result, Score, Notes)       
       echo "<table width=90% border=\"1\" align=\"center\">";
       echo "<tr><th></th><th>QUERY</th><th>Sample Solution</th><th>RESULT</th><th>Score</th><th>Notes</th></tr>\n";
       
       $escape_str = "";

       # For each Query Print:
       #	- link to student's solution with given input
       #	- expected result (link to sample solution)
       #	- extracted student's result for current query
       #	- score based on matching solution and output
       #	- comment based on score (displayed only if answer incorrect)
       #		(describes type of test case)
       ###################################################
       for ($i = 0; $i < count($queries); $i++)
       {
       	       $temp = $i + 1;
	       echo "<tr>";
	       echo "<td>$temp</td>\n";

	       # link to student's solution with given input
	       echo "<td>\n";
	       $escape_str =  urlencode("$queries[$i]");
	       echo "<a class=\"php_editable\" href=\"$submissions_directory/$sid/$default_php?expr=$escape_str\" target=_blank > $queries[$i] </a>\n";
	       echo "</td>\n";

	       # expected result (link to sample solution)
	       echo "<td>\n";
	       $escape_str = urlencode("$queries[$i]");
	       echo "<a class=\"sampleresult\" href=\"$sample_php?expr=$escape_str\" target=_blank > $solutions[$i] </a>\n";
	       echo "</td>\n";
	       
	       # extract student's result for current query
	       $escape_str = urlencode("$queries[$i]");
	       echo "<td class=\"phpresult\" id=\"$submissions_directory/$sid/$default_php?expr=$escape_str\">\n";
	       echo "<script type=\"text/javascript\">\n";
	       echo " document.write(get_result(\"$submissions_directory/$sid/$default_php?expr=$escape_str\"));\n";
	       echo "</script>\n";
	       echo "</td>\n";

	       # score based on matching solution and output
	       echo "<td class=\"qscore\" onClick=\"editCell(this, 'number', '$sid');\">0</td>\n";

	       # comment based on score (displays test case description if solution wrong)
#	       echo "<td class=\"qnotes\" onClick=\"editCell(this, 'text', '$sid');\">$descriptions[$i]</td>\n";
	       echo "<td class=\"qnotes\">$descriptions[$i]</td>\n";
	       echo "</tr>\n";
       }
       ###################################################
     
       echo "<span><tr>\n";
       echo "<td colspan=4 style=\"font-weight:bold\" >Total Score</td>\n";
       echo "<td class=\"tscore\" style=\"font-weight:bold\" ></td>\n";
       echo "<td class=\"num_correct_score\" style=\"font-weight:bold\" ></td>\n";
       echo "</tr>\n";
       
       ## Master NOTES: Combines notes from all queries for current student
       ## and is editable by user
       echo "<tr><td colspan=6 class=\"tnotes\" onClick=\"editCell(this, 'text', '$sid');\">Notes:</td></tr></span>\n";

       echo "</table>\n";
       echo "</div>\n";

}
?>

<p align=center>*****************************************************************</p>

<!-- Show All / Hide All Buttons -->
<div align=center><a class="button"  style="width:75" href="javascript:show_all('<span>Show</span>'"/><span>Show All</span></a>
<a class="button"  style="width:75" href="javascript:hide_all('<span>Hide</span>');"/><span>Hide All</span></a></div>

</body></html>
