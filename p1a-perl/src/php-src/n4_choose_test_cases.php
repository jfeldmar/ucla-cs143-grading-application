<?php

###################################################
## Choose test cases
## 0. Save list of submissions to grade from previous form (n3) into file
## 1. Load Default test cases
## 2. Javascript interface to add/delete/save test cases
## 2.1 Save options:
##	- load: use test case for this run of the grading program only
##	- save: use test case for this run and save as "new" default test cases
###################################################

#**********GLOBAL VARIABLES***********
	$CSS_file = "../html-css/styleSheet.css";

	$sids_to_grade_file = "../logs/SIDstoGrade.csv";
	$query_directory = "../test_cases/queries";
	$solutions_directory = "../test_cases/solutions";
	$descriptions_directory = "../test_cases/descriptions";
#*************************************

echo header('Content-type: text/html');
?>

<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
<link rel="stylesheet" type="text/css" href="<?php echo $CSS_file; ?>" />

<script type="text/javascript">

// add Selected test case to list
function addSelection()
{
	var optn = document.createElement("OPTION");
	optn.text = prompt("Type in your test query", "2+3");
	if (optn.text != false){
		var soltn = prompt("Confirm or edit solution", eval(optn.text));
		if (soltn != null){
			var descr = prompt("Enter test case description <br/>(no commas)", "testing order of operations");
			if (descr.indexOf(",") >= 0) {alert("Commas Not Allowed in Description");}
			else if (descr != false){
				optn.value = optn.text + "," + soltn + "," + descr.replace('"', '\"');
				optn.text = optn.text + " (SOLN: " + soltn + ", DESCR: " + descr.replace('"', '\"') + ")";
				document.getElementById("tests").options.add(optn);
			}
		}
	}
}

// remove Selected test case from list
function deleteSelection()
{
	var opts = document.getElementById("tests").options;
	for (var i = opts.length-1; i >= 0; i--)
	{
		if (opts[i].selected)
		{
			document.getElementById("tests").removeChild(opts[i]);
		}
	}	
}

// highlight selected (clicked on) option
function select(opt)
{
	var sel = document.getElementById("tests").options.selectedIndex;
	document.getElementById("tests").options[sel].className = "selected";
}

// submit selected testcase in selected mode (load or save)
function choosesubmit(state)
{
	if (state == 'load')
	{
		alert("using these test cases for this run only");
		
	}
	if (state == 'save')
	{
		alert("warning: default test cases will be overwritten");
	}
	document.getElementById("savetype").value = state;
	selectAll();
	document.getElementById("myform").submit();
}

// select all test cases
function selectAll()
{
	var opts = document.getElementById("tests").options;
	for (var i = 0; i < opts.length; i++)
	{
		opts[i].selected = true;
	}	
}

// deselect all test cases
function deselectAll()
{
	var opts = document.getElementById("tests").options;
	for (var i = 0; i < opts.length; i++)
	{
		opts[i].selected = false;
	}	
}
</script>

</head>

<body onload="javascript:deselectAll()">

<h1>CS143 - Project 1A Grading Application</h1>
<h2 align=center> Confirm/Add/Delete/Save Test Cases </h2>

<?php

## 0. Save list of submissions to grade from previous form into file
###################################################

$FH = fopen("$sids_to_grade_file", 'w+') or die("Unable to open/create $sids_to_grade_file file");
foreach ($_POST['check'] as $check)
{
	fwrite($FH, "$check\n");
}
fclose($FH);


## 1. Load Default test cases
###################################################

## 1.1 Check that queries and solutions directories exists
if (!file_exists($query_directory) or !file_exists($solutions_directory)){
	echo "<p class=error>ERROR: Unable to locate test case directories: $query_directory	AND/OR	$solutions_directory director</p>";
}


## 1.2 Read default queries, solutions, and descriptions
$qfiles = array();
$sfiles = array();
$dfiles = array();

$QDIR = opendir("$query_directory") or die("Can't open query directory $query_directory");
    while ( $f = readdir($QDIR))
    {
    	if ($f != "." && $f != "..")
		array_push($qfiles, $f);
    }
closedir($QDIR);

$SDIR = opendir("$solutions_directory") or die("Can't open solutions directory $solutions_directory");
    while ( $f = readdir($SDIR))
    {
    	if ($f != "." && $f != "..")
		array_push($sfiles, $f);
    }
closedir($SDIR);

$DDIR =opendir("$descriptions_directory") or die("Can't open descriptions directory $descriptions_directory");
    while ( $f = readdir($DDIR))
    {
    	if ($f != "." && $f != "..")
		array_push($dfiles, $f);
    }
closedir($DDIR);
?>

<FORM id="myform" method=POST action="../php-src/n5-run-test.php">
<p align=center>

<!-- Links to edit test cases list-->
<a class=button style="width:100" href="#" onClick="javascript:addSelection();"><span>Add Item</span></a>
<BR/><a class=button style="width:100" href="#" onClick="javascript:deleteSelection();"><span>Delete Item</span></a>
</p>

<TABLE style=none align=center border=0><tr><th>Query</th></tr><tr><td>
<SELECT id="tests" name="tests[]" MULTIPLE SIZE=10>

<?php

$soltns = array();
$descr = array();

// foreach query: display query, solution, and description
foreach ($qfiles as $query)
{
	$QFILE = fopen("$query_directory/$query", 'r') or next;
	$q = fread($QFILE, filesize("$query_directory/$query"));
	fclose($QFILE);
	
	$SFILE = fopen("$solutions_directory/$query", 'r') or next;
	$s = fread($SFILE, filesize("$solutions_directory/$query"));
	fclose($SFILE);

	$DFILE = fopen("$descriptions_directory/$query", 'r') or next;
	$d = fread($DFILE, filesize("$descriptions_directory/$query"));
	fclose($DFILE);
	
	echo "\n<OPTION class='options' onclick='select(this);' VALUE=\"$q,$s,$d\" >$q (SOLN: $s, DESCR: $d)</OPTION>\n";
}

?>

</SELECT>
</td></tr>
</TABLE>

<!-- Submit page with LOAD or SAVE option-->
<input type=hidden id="savetype" name="savetype" value=""/>
<BR/><BR/><a class=button style="width:200" href="#" onClick="choosesubmit('load')" /><span>Save Test Cases</span></a>
<BR/><BR/><a class=button style="width:300" href="#" onClick="choosesubmit('save')" /><span>Save Test Cases as Default Test Cases</span></a>
</FORM>
</body></html>
