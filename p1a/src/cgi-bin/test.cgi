#!/usr/bin/perl -wT

use CGI ':standard';
use CGI::Carp qw(fatalsToBrowser);  

my $files_location;  
my $ID;  
my @fileholder;

$files_location = "../logs";

$ID = "SavedGradingFile.csv";

if ($ID eq '') {  
print "Content-type: text/html\n\n";  
print "You must specify a file to download.";  
} else {

open(DLFILE, "<$files_location/$ID") || Error('open', 'file');  
@fileholder = <DLFILE>;  
close (DLFILE) || Error ('close', 'file');  

open (LOG, ">../logs/test.log") || Error('open', 'file');
print LOG "$ID\n";
close (LOG);

print "Content-Type:application/x-download\n";  
print "Content-Disposition:attachment;filename=$ID\n\n";
print @fileholder
}
