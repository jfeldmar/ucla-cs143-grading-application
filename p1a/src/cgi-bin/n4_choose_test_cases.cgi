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

$CGI::POST_MAX = 1024 * 1000 * 10; #max 10 MB
$CGI::DISABLE_UPLOADS = 0;
my $query = new CGI;
my $sids_to_grade_file = "../logs/SIDstoGrade.csv";
my $query_directory = "../test_cases/queries";
my $solutions_directory = "../test_cases/solutions";

print "Content-type: text/html\n\n";
print <<ENDHTML;
<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />

<script type="text/javascript">
function addSelection()
{
	var optn = document.createElement("OPTION");
	optn.text = prompt("Type in your test query", "2+3");
	if (optn.text != false){
		var soltn = prompt("Confirm or edit solution", eval(optn.text));
		if (soltn != null){
			optn.value = optn.text + "," + soltn;
			optn.text = optn.text + " (SOLN: " + soltn +")";
			document.getElementById("tests").options.add(optn);
		}
	}
}
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
function select(opt)
{
	var sel = document.getElementById("tests").options.selectedIndex;
	document.getElementById("tests").options[sel].className = "selected";
}

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
function selectAll()
{
	var opts = document.getElementById("tests").options;
	for (var i = 0; i < opts.length; i++)
	{
		opts[i].selected = true;
	}	
}
</script>
</head>
<body>

<h1>CS143 - Project 1A Grading Application</h1>
<h2 align=center> Confirm/Add/Delete/Save Test Cases </h2>
ENDHTML

###################################################
## Choose test cases
## 0. Save list of submissions to grade from previous form******************
## 1. Load Default test cases
## 2. Javascript interface to add/delete/save test cases
###################################################

## 0. Save list of submissions to grade from previous form******************
###################################################
my @selected = $query->param("check") or warn("No File Selected");

open FH, ">$sids_to_grade_file" or die("Unable to open/create $sids_to_grade_file file: $!");
foreach (@selected){
	print FH "$_\n";
}
close FH;



## 1. Load Default test cases
###################################################

## 1.1 Check that queries and solutions directories exists
unless ( -d $query_directory and -d $solutions_directory){
	print qq(<p class=error>ERROR: Unable to locate test case directories: $query_directory	AND/OR	$solutions_directory director</p>);
}

## 1.2 Read default queries and solutions
opendir QDIR, "$query_directory" or die("Can't open query directory $query_directory: $!");
my @qfiles = grep { $_ ne '.' && $_ ne '..' } readdir QDIR;

opendir SDIR, "$solutions_directory" or die("Can't open solutions directory $solutions_directory: $!");
my @sfiles = grep { $_ ne '.' && $_ ne '..' } readdir SDIR;

print qq(\n<form id="myform" method=GET action="../cgi-bin/n5-run-test.cgi">\n);
print qq(<p align=center>);
print qq(<a class=button style="width:100" href="#" onClick="javascript:addSelection();"><span>Add Item</span></a>\n);
print qq(<BR/><a class=button style="width:100" href="#" onClick="javascript:deleteSelection();"><span>Delete Item</span></a>\n);
print qq(</p>);

print qq(<p align=center>\n);
print qq(<SELECT id="tests" name="tests" MULTIPLE SIZE=10>\n);

foreach my $query (@qfiles)
{
	open QFILE, "$query_directory/$query" or next;
	chomp(my $q = <QFILE>);
	close QFILE;
	
	open SFILE, "$solutions_directory/$query" or next;
	chomp(my $s = <SFILE>);
	close SFILE;
	
	print qq(\n<OPTION onclick="select(this);" VALUE="$q,$s" >$q (SOLN: $s)</OPTION>\n);
}

print qq(</SELECT>);
print qq(</p>\n);

print qq(<input type=hidden id="savetype" name="savetype" value=""/>\n);
print qq(<BR/><BR/><a class=button style="width:200" href="#" onClick="choosesubmit('load')" /><span>Save Test Cases</span></a>\n);
print qq(<BR/><BR/><a class=button style="width:300" href="#" onClick="choosesubmit('save')" /><span>Save Test Cases as Default Test Cases</span></a>\n);
print qq(</form>\n);
print qq(</body></html>);
