<?php

###################################################
## save form data from previous page (i.e. SIDs for submissions to grade) into a log
## displays results of running test cases on selected submissions
## provides buttons to save log file of results as well as MyUCLA Gradebook compatible file
## provides interface to change scores and notes for each student
## provides interface to change which file from a student's submission is graded
###################################################

#**********GLOBAL VARIABLES***********
	$CSS_file = "../html-css/styleSheet.css";

	$test_cases_dir = "../test_cases";
	$query_directory = "../test_cases/queries";
	$solutions_directory = "../test_cases/solutions";
	$descriptions_directory = "../test_cases/descriptions";
	$default_data_tarfile = "../default-data/test-cases.tar";
	$sids_to_grade_file = "../logs/SIDstoGrade.csv";
	$sid_name_file = "../logs/sid-name.txt";

	$default_php = "calculator.php";
	$sample_php = "../file-uploads/sampleCalculator.php";
	$submissions_directory = "../submissions";
	$temp_directory = "../temp";
	$pop_up_window = "../html-css/choosefile-popup.html";
	
	$download_tsv_file_link = "n6-downloadTSV.php";
	$download_log_file_link = "n7-downloadLOG.php";
	
	$max_pts = 10;
#*************************************

echo header('Content-type: text/html');
?>

<html><head>
<title>CS143 - Project 1A Grading Application</title>
	

	<link rel="stylesheet" type="text/css" href="<?php echo $CSS_file; ?>" />

</head>

<?php echo "<body onload=\"max_pts = $max_pts;load_totals('".$default_php."');\">"; ?>
	
<script  type="text/javascript">
	temp = "we are still testing";
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
	echo "unique_sids = [" . implode( ",", $unique_sids) . $last . "];\n";

	#load SID-Name Array File into Javascript Array/Hash Table
	$sids_names = unserialize(file_get_contents($sid_name_file));
	echo "sidsnames = [];\n";
	foreach ($sids_names as $key => $value)
	{
		echo "sidsnames['$key'] = '$value';\n";
	}

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
		$declaration = "files_$sid = [";

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
		$declaration .= "];";

		print "$declaration\n";
	}
	?>
</script>

<script type="text/javascript" src="../html-css/js/n5-script-functions.js"></script>
<script type="text/javascript" src="../html-css/js/wz_tooltip.js"></script>

<h1>CS143 - Project 1A Grading Application</h1>
<p align=center><input type=button value='Restart' onClick="location.href='../html-css/start.html'"/></p>

<h2> === Step 5. Load/Save Test Cases & Run Tests === </h2>

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
<tr><th></th><th>Test Input</th><th>Solution</th><th>Description</th><th>Points</th></tr>

<?php
#Display test case table
$count = 1;
$queries =array();
$solutions =array();
$descriptions =array();
$iterator = 0;

foreach ($formdata as $d){
	$iterator += 1;
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
	# do not use the dynamic max_score update here - it is not yet functional
#	echo "<td id=query_max_score_$iterator onClick=\"change_pts(this);\">$max_pts</td>\n";
	echo "<td id=query_max_score_$iterator >$max_pts</td>\n";
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
for ($i = 0; $i < count($queries); $i++)
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

if (file_exists($sample_php)){
	echo "<p align=center><a href=\"$sample_php\" target=_blank>Your Sample Submission</a> &nbsp;&nbsp;&nbsp;\n";
	echo "(<a href=\"$sample_php?expr=2%2B3\" target=_blank>Simple Test Case: 2+3</a>)</p>\n";
	echo "<hr/>\n";
}

## 3. Display table summarizing test case output 
##	for selected submissions (use log for reference)
###################################################

?>

<!-- Link/Form to download TSV containing final grades -->
<form action="<?php echo $download_tsv_file_link ?>" method="POST" name=gettsv >
<input type=hidden id ="tsv_data" name="tsv_data" value=""/></p>
<div align=center><a class="button" style="width:200" href="javascript:javascript:submit_tsv(this);"><span>DOWNLOAD TSV</span></a></div>
</form>

<!-- Link/Form to download HTML containing data on current page (grading information) -->
<form action="<?php echo $download_log_file_link ?>" method="POST" name=getlog >
<input type=hidden id ="log_data" name="log_data" value=""/></p>
<div align=center><a class="button" style="width:200" href="javascript:javascript:submit_log(this);"><span>DOWNLOAD LOG</span></a></div>
</form>

<!-- Table Containing Submissions Data -->
<div id="table-data">
<div align=center>

<!-- Show All / Hide All Buttons -->
<a class="button"  style="width:75" href="javascript:show_all('<span>Show</span>');"/><span>Show All</span></a>
<a class="button"  style="width:75" href="javascript:hide_all('<span>Hide</span>');"/><span>Hide All</span></a>
</div>

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
       echo "&nbsp;&nbsp;Test Case Results for SID $sid ($sids_names[$sid])\n";
       echo "<a class=button href=\"#\"  style=\"width:110\" onclick=\"javascript:update_total_score('$sid');\" ><span>Sum Scores</span></a>\n";
#       echo "<a class=button href=\"#\"  style=\"width:105\" onclick=\"javascript:update_notes('$sid');\" ><span>Concat Notes</span></a></p>\n";

#       echo "<input type=\"button\" onclick=\"javascript:update_total_score('$sid');\" value=\"Sum Scores\" >\n";
#       echo "<input type=\"button\" onclick=\"javascript:update_notes('$sid');\" value=\"Concat Notes\" >\n";

       echo "<div class=\"submissions\" id=\"$sid\" title=\"$sids_names[$sid]\" style=\"overflow:hidden;display:none\">\n";

	## Name of/ Link to file being graded now
       echo "<p align=center>Graded File: <a class=\"link_editable\" href=$submissions_directory/$sid/$default_php target=_blank > $default_php </a>\n";
        ## Button to change file being graded
       echo "&nbsp;or&nbsp;<a class=button style=\"width:200\" href=\"#\" onclick=\"ChooseFilePopUp('$pop_up_window','$sid', files_$sid);\"/><span>Choose PHP File to Grade</span></a></p>\n";

	## Query Table (Student's solution, Sample Solution, Student Result, Score, Notes)       
       echo "<table width=90% border=\"1\" align=\"center\">";
       echo "<tr><th></th><th>QUERY</th><th>Sample Solution</th><th>STUDENT RESULT</th><th>Score</th><th>Notes</th></tr>\n";
       
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
</div>
</body></html>
