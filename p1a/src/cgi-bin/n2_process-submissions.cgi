#!/usr/bin/perl -w

###################################################
#  This script processes the form submitted in n1_initialize.cgi
#1. Uploads compressed tar file of submissions to $upload_dir/$zip_filename
#2. Uploads sample PHP calculator solution to $upload_dir/$php_filename
#3. Extract compressed tar file into submissions directory
###################################################

use strict;
use CGI;
use CGI::Carp qw ( fatalsToBrowser );
use File::Basename;

$CGI::POST_MAX = 1024 * 1000 * 50; #max 50 MB
$CGI::DISABLE_UPLOADS = 0;
my $query = new CGI;

my $safe_filename_characters = "a-zA-Z0-9_.-";
my $upload_dir = "../file-uploads";
my $zip_filename = "project1A-Submissions.tar";
my $php_filename = "sampleCalculator.php";

#
#HTML
###################################################

print "Content-type: text/html\n\n";
print <<ENDHTML;
<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
</head>
<body>
<h3 align=left><a href="n1_initialize.cgi">BACK (all files from this run will be deleted)</a></h3>
<h1>CS143 - Project 1A Grading Application</h1>
<h2> Processing Uploaded Files...</h2>
ENDHTML

#
#Load form parameters
###################################################

my $zipfile = $query->param("student-submissions") or die("No File Selected");
my $webhostdir = $query->param("webhost-location");
my $sampleCalcFile = $query->param("calculator.php");

if ( !$zipfile )
{
 print $query->header ( );
 print "There was a problem uploading your submissions zip file (try a smaller file).";
 exit;
}


#
#Check for correct file extension (*.tar)
###################################################

my ( $zip_name, $zip_path, $zip_extension ) = fileparse ( $zipfile, '\..*' );

if (lc($zip_extension) ne ".tar")
{
	 die "Submissions compressed file filename must be a .tar file";
}

#
#Upload .tar file (submissions for Project 1A)
###################################################

my $upload_filehandle = $query->upload("student-submissions") or die "Cannot open $zipfile: $!";


open ( UPLOADFILE, ">$upload_dir/$zip_filename" ) or die "$!";
binmode UPLOADFILE;

print "<p>Opened Zip File...</p>";

print "<p>uploading zip file...</p>";
while ( <$upload_filehandle> )
{
	print UPLOADFILE;
} 

close UPLOADFILE; 

#
#Print file size and upload status
###################################################

my $filesize = -s "$upload_dir/$zip_filename";
print "<p>Size of zipfile: " . $filesize . "</p>"; 
print "<p>uploaded zip file.</p>";

#
#Upload sample PHP Calculator Solution
###################################################

if ( $sampleCalcFile )
{
	print "<p>Processing Sample Calculator PHP file upload...</p>";
	
	#
	#Check for correct file extension (*.tar)
	#
	my ( $php_name, $php_path, $php_extension ) = fileparse ( $sampleCalcFile, '\..*' );

	if ($php_extension ne ".php")
	{
	 die "Submitted Sample Calculator PHP file filename must be a .php file";
	}

	#
	#Upload .php file (sample calculator solution)
	#

	$upload_filehandle = $query->upload("calculator.php") or die "Cannot open $zipfile: $!";


	open ( UPLOADFILE, "+>$upload_dir/$php_filename" ) or die "$!";
	binmode UPLOADFILE;

	print "<p>Opened PHP File...</p>";

	print "<p>uploading PHP file...</p>";
	while ( <$upload_filehandle> )
	{
		print UPLOADFILE;
	} 

	close UPLOADFILE; 

	#
	#Print file size and upload status
	#

	my $filesize = -s "$upload_dir/$php_filename";
	print "<p>Size of PHP file: " . $filesize . "</p>"; 
	print "<p>uploaded PHP Sample Calculator Solution file.</p>";
	
	#
	# 3. extract into submissions directory
	#
	!system("tar -C ../submissions/ -xf $upload_dir/$zip_filename") or die("Unable to Extract compressed submissions $upload_dir/$zip_filename into ../submissions/ : $!");


}

print '<p align=right><a href="n3_error_process_uploads.cgi" >Continue</a></p>';
print "</html>";

