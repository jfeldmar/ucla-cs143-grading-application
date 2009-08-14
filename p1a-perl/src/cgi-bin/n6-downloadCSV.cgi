#!/usr/bin/perl -w

###################################################
## generate a CSV (comma separated value) file for user to download
## saves SID, total score, and total notes
###################################################

# Uncomment lines and change Content-Type to print file to screen

use CGI;
use Text::CSV;

use CGI::Carp qw(fatalsToBrowser);  
my $query = new CGI;

my $file = CGI::unescape($query->param("csv_data"));
my $size = CGI::unescape($query->param("csv_size"));
my $fileholder = "";


#print qq(<p>Passed array width: $size </p>);
#print qq(<p>Passed data: $file </p>);

my $mycsv = Text::CSV->new();
my @entries = $mycsv->parse($file);
my @fields = $mycsv->fields();

# Uncomment lines and change Content-Type to print file to screen
#print qq(<pre>);
for (my $i = 0; $i <= $#fields; $i++)
{
	for (my $j = 0; $j < $size; $j++)
	{
		$fields[$i] =~ s/"/""/g;
		$fileholder .= "\"$fields[$i]\"";
		if ( $j != $size-1){
			$fileholder .= ",";
		}	
#		print "\"$fields[$i]\",";
		$i++;
	}
	$i--;
	$fileholder .= "\n";
#	print "\n\r";
}
#print qq(</pre>);

# Write file to log (deletes previously downloaded file)
open (LOG, ">../logs/SavedGradingFile.csv") || warn("Content-type: text/html\n\nThe server can't open ../logs/SavedGradingFile.csv: $! \n");
print LOG "$fileholder\n";
close (LOG); 

print "Content-Type:application/x-download\n";  
print "Content-Disposition:attachment;filename=GradesCSV.csv\n\n";
print $fileholder
