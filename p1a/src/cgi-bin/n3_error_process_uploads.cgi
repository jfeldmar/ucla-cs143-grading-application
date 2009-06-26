#!/usr/bin/perl -w

###################################################
# extract provided zip file, confirm correct file hierarchy
# upload/link to sample calculator solution, confirm functionality
###################################################
use strict;

print "Content-type: text/html\n\n";
print <<ENDHTML;
<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
</head>
<body>
<h1>CS143 - Project 1A Grading Application</h1>
ENDHTML
#<h2> checking if the uploaded files worked</h2>

#Check that the Text::CSV Perl Module is installed
###################################################
eval "use Text::CSV";
eval "use HTML::TreeBuilder";
if ($@)
{
	print "<p>*******NO TEXT::CSV or HTML::TreeBuilder MODULE DETECTED*******</p>";
	print "<p>Run the following commands in shell as root to acquire the Text::CSV or HTML::TreeBuilder Module(s)</p>";
	print "<p>(Internet Connection Required)</p>";
	print "<p>>perl -MCPAN -e shell</p>";
	print "<p>>install Text::CSV</p>";
	print "<p>>install HTML::TreeBuilder</p>";
	print "<p>Reload Page after Module Installation complete</p>";
	print "</body></html>";
	print exit;
}
else
{
	use CGI;
	use CGI::Carp qw ( fatalsToBrowser );
	my $query = new CGI;

	my $safe_filename_characters = "a-zA-Z0-9_.-";
	my $upload_dir = "../file-uploads";
	my $zip_filename = "project1A-Submissions.tar";
	my $php_filename = "sampleCalculator.php";
	my $submissions_directory = "../submissions";
	my $submissions_csv_file = "../submissions/submission.csv";

	# If sample Calculator PHP solution was uploaded by user
	# Display a link to it
	if (-f "$upload_dir/$php_filename")
	{
		###################################################
		#Sample Submission Testing
		# 1. Result has correct name tag
		# 2. Link to simple test case
		###################################################
		find_attribute("$upload_dir/$php_filename", "name", "expr");

		print "<p>Please Test:&nbsp;&nbsp;&nbsp;";
		print "<a href=\"$upload_dir/$php_filename\" target=_blank>Your Sample Submission</a> * ";
		print "<a href=\"$upload_dir/$php_filename?expr=2%2B3\" target=_blank>Simple Test Case: 2+3</a></p>";

	}


	###################################################
	# Tar file of submissions
	# 1. extract submissions file into "submissions" directory
	# 2. Display files generated (missing files, missing ID tag,etc.)
	# 3. Allow user to generate list of programs to grade
	###################################################
	#1. extract into submissions directory

	!system("tar -C ../submissions/ -xf $upload_dir/$zip_filename ") or die("Unable to Copy Default Test Cases: $!");


	# 2. Display files generated (missing files, missing ID tag,etc.)
	# import submissions.csv

	my $csv_submissions = Text::CSV->new();
	open FILE, $submissions_csv_file or die("Can't open $submissions_csv_file file: $!");
	my $total_rows = 0;
	
	print "<table border=1>";
	
	while(my $line = <FILE>)
	{
            if($csv_submissions->parse($line))
            {
		my @fields = $csv_submissions->fields();
        	print qq(<tr id=$fields[0]> \n);
		print qq(\t <td>$fields[0]</td> \n);
		print qq(\t <td>$fields[1]</td> \n);
		#print link to file source (add red X if no attribute found)
		opendir DIR, "$submissions_directory/$fields[0]" or warn("Can't open submission directory for $fields[0]:$fields[1]: $!");
		my @files = grep { $_ ne '.' && $_ ne '..' } readdir DIR;
		print qq(\t <td> \n);
		print join("\n <BR/>",@files),"<BR/>";
#		print(@files, "<br />");
		print qq(\t </td> \n);
		closedir DIR;
		#print link to Calculator solution (add red X if no attribute found)
        	print qq(</tr> \n);
            }
	}	
	close FILE;
	
	print "</table>";
	
	# check submissions for errors

	# generate list of students to grade (using user input)
	
	print "</body></html>";
}

sub say_hello {
	print "<p>Say Hello!!!</p>";
}

sub find_attribute{
	my ($source_file, $attr, $value) = @_;
	my $mytree = HTML::TreeBuilder->new;
	$mytree->parse_file("$source_file");
	my @name_tags = $mytree->find_by_attribute($attr, $value);

	unless(@name_tags){
		print "<p>Please update the solution: $source_file</p>";
		print "<p>A tag with attribute $attr=$value NOT found. </p>";
		print "<p>The &lt;input&gt; tag which takes the calculator input must have ";
		print " the attribute $attr=$value</p>";
		exit;
		return 0;
	}

	$mytree->delete;	
	return 1;
}
