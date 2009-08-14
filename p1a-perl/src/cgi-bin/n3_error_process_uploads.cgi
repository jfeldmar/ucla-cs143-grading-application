#!/usr/bin/perl -w

###################################################
## extract provided zip file, confirm correct file hierarchy
## upload/link to sample calculator solution, confirm functionality
## display/link to all students' submissions
###################################################
use strict;
	my $submitted_php_extension = "php";
	my $submitted_txt_extension = "txt";

	my $safe_filename_characters = "a-zA-Z0-9_.-";
	my $upload_dir = "../file-uploads";
	my $zip_filename = "project1A-Submissions.tar";
	my $php_filename = "sampleCalculator.php";
	my $submissions_directory = "../submissions";
	my $temp_directory = "../temp";
	my $html_images_dir = "../html-css/images/";
	my $editable_src = "editable_src";
	my $submissions_csv_file = "../submissions/submission.csv";


print "Content-type: text/html\n\n";
print <<ENDHTML;
<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
	<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />
</head>
<body>
<script type="text/javascript" src="../html-css/js/wz_tooltip.js"></script>
<script type="text/javascript" src="../html-css/js/checkAll.js"></script>
<h1>CS143 - Project 1A Grading Application</h1>
ENDHTML

##Check that required Modules are installed
###################################################
use File::Path;
use HTML::Entities;
eval "use Text::CSV";
eval "use HTML::TreeBuilder";
if ($@)
{
	print "<p>*******NO TEXT::CSV or HTML::TreeBuilder or File::Path or HTML::Entities MODULE DETECTED*******</p>";
	print "<p>Run the following commands in shell as root to acquire the Text::CSV or File::Path or HTML::Entities or HTML::TreeBuilder Module(s)</p>";
	print "<p>(Internet Connection Required)</p>";
	print "<p>>perl -MCPAN -e shell</p>";
	print "<p>>install Text::CSV</p>";
	print "<p>>install HTML::TreeBuilder</p>";
	print "<p>>(insert missing module name here)</p>";
	print "<p>Reload Page after Module Installation complete</p>";
	print "</body></html>";
	print exit;
}
else
{
	use CGI;
	use CGI::Carp qw ( fatalsToBrowser );
	my $query = new CGI;

	# If sample Calculator PHP solution was uploaded by user
	# Display a link to it
	if (-e "$upload_dir/$php_filename")
	{
		###################################################
		## Sample Submission Testing
		## 1. Result has correct name tag
		## 2. Link to simple test case
		###################################################

		print "<p>Please Test:&nbsp;&nbsp;&nbsp;";
		print "<a href=\"$upload_dir/$php_filename\" target=_blank>Your Sample Submission</a> * ";
		print "<a href=\"$upload_dir/$php_filename?expr=2%2B3\" target=_blank>Simple Test Case: 2+3</a>";

		if (find_attribute("$upload_dir/$php_filename", "name", "expr")){
		     print qq(&nbsp;<img src="$html_images_dir/greenCheck.gif" onmouseover="Tip('OK: Attribute name=expr found')" onmouseout="UnTip()"/> </p>);
		}else{
		     print qq(&nbsp;<img src="$html_images_dir/redX.gif" class=error onmouseover="Tip('ERROR: Attribute name=expr NOT found; unable to process file')" onmouseout="UnTip()" /></p>);
		}
	}

	###################################################
	## Tar file of submissions
	## 1. Display files generated (missing files, missing ID tag,etc.)
	## 2. copy each student's submitted files into a new subdirectory
	##	to allow for easier preview and editing of submitted source code
	## 3. Allow user to generate list of programs to grade
	###################################################

	# delete editable_src directory in temporary directory (clean up)
	if (-d "$temp_directory/$editable_src"){
		rmtree("$temp_directory/$editable_src",0, 0 ) or die("Cannot remove contents of $temp_directory/$editable_src directory: $!");		
	}
	mkdir("$temp_directory/$editable_src") or die "Cannot create $temp_directory/$editable_src directory: $!";
		
	# 1. Display files generated (missing files, missing ID tag,etc.)

	# import submissions.csv
	my $csv_submissions = Text::CSV->new();
	open FILE, $submissions_csv_file or die("Can't open $submissions_csv_file file: $!");
	my $total_rows = 0;

	print qq(<form name=selectSID method="POST" action="../cgi-bin/n4_choose_test_cases.cgi">);

	# form submit button
	print qq(<p align=center><a class=button style="width:150pt" href="#" onclick="document.selectSID.submit()" ><span>Next (select test cases)</span></a></p>);
	
	# select all button
	print qq(<p align=center><a class=button href="#" style="width:90" onclick="javascript:checkAll()" ><span>Select All</span></a></br>);
	# deselect all button
	print qq(<a class=button href="#" style="width:92" onclick="javascript:uncheckAll()" ><span>Deselect All</span></a></p>);
	
	# note to user
	print "<div align=center><font size=2>(some students have multiple submissions)</font></div>";
	
	print "<div align=center><table>";
	print qq(<tr><th></th><th>SID</th><th>Name</th><th>Source Files</th><th>Interact with Code</th></tr>);

	while(my $line = <FILE>)
	{
            if($csv_submissions->parse($line))
            {
		my @fields = $csv_submissions->fields();
		my $sid = $fields[0];
		my $name = $fields[1];
		#begin table row/checkbox
        	print qq(<tr id=$sid> \n);		#prints id=SID for <tr> tag "id"
		print qq(\t <td><input type=checkbox name="check" value=$sid></td> \n);
		print qq(\t <td>$sid</td> \n);		#prints SID
		print qq(\t <td>$name</td> \n);		#prints Name
		
		#print link to file source (add red X if no attribute found)
		opendir DIR, "$submissions_directory/$sid" or die("Can't open submission directory for $sid:$name: $!");
		my @files = grep { $_ ne '.' && $_ ne '..' } readdir DIR;

		## 2.1 create for each student's submitted files a new (editable_src) directory in the temporary directory
		##	to allow for easier preview and editing of submitted source code
#		if (-d "$temp_directory/$editable_src/$sid"){
#			rmtree("$temp_directory/$editable_src/$sid",0, 0 ) or die("Cannot remove contents of $temp_directory/$editable_src/$sid directory: $!");		
#		}
#		mkdir("$temp_directory/$editable_src/$sid") or die "Cannot create $temp_directory/$editable_src/$sid directory: $!";
	

		# list links to submitted files
		print qq(\t <td> \n);
		foreach my $submitted_file (@files){
			#print only files, not directories
			unless (-d "$submissions_directory/$sid/$submitted_file"){
			     if ($submitted_file =~ m/.$submitted_php_extension$/){
			     	     my $error;
			     	     my $tooltip;
				     if ( find_attribute("$submissions_directory/$sid/$submitted_file", "name", "expr") ){
					$error = 0;
					$tooltip = qq(onmouseover="Tip('OK: Attribute name=expr found')" onmouseout="UnTip()");
				     }else{
					$error = 1;
					$tooltip = qq( class=error onmouseover="Tip('ERROR: Attribute name=expr NOT found; unable to process file')" onmouseout="UnTip()");
				     }
	
				     print qq(<a href="$submissions_directory/$sid/$submitted_file" target="_blank" $tooltip >$submitted_file</a> \n);

				     if ($error){
					  print qq(<img src="$html_images_dir/redX.gif" $tooltip /><BR/>);
				     }else{
					  print qq(<img src="$html_images_dir/greenCheck.gif" $tooltip/> <BR/>);
				     }
			     }elsif ($submitted_file =~ m/.$submitted_txt_extension$/){
				     print qq(<a href="$submissions_directory/$sid/$submitted_file" target="_blank" >$submitted_file</a> \n);
				     print " </BR>";
			     }
			}
		}
		print qq(\t </td> \n);
		closedir DIR;
		
		#print link to Calculator solution (add red X if no attribute found)
		# add green check if required attribute value found
		print qq(\t <td> \n);
		foreach my $submitted_file (@files){
			#print only files, not directories
			unless (-d "$submissions_directory/$sid/$submitted_file"){
				open SUBFILE, "$submissions_directory/$sid/$submitted_file" or die("Can't open $submitted_file for $sid:$name: $!");
				chomp(my @lines = <SUBFILE>);
				my $source = join("\n", @lines);
				close SUBFILE;
				
				## 3.2 copy each student's submitted files into a new (editable_src) subdirectory
				##	to allow for easier preview and editing of submitted source code
	#			open CFILE, ">$temp_directory/$editable_src/$sid/$submitted_file" or die "unable to create file $submissions_directory/$sid/$editable_src/$submitted_file: $!";
	#			# reformat text to display php code literally (without execution)
	#			if ($submitted_file =~ m/.$submitted_php_extension$/){
	#				$source = encode_entities($source);
	#				$source = "<pre>".$source."<pre>";
	#			}
	#			print CFILE $source;
	#			close CFILE;
				
				if ($submitted_file =~ m/.$submitted_php_extension$/){
					print qq(<a href="$temp_directory/$editable_src/$sid/$submitted_file" target="_blank">Source: $submitted_file</a><BR/> \n);
				}elsif ($submitted_file =~ m/.$submitted_txt_extension$/){
					print qq(<a href="$temp_directory/$editable_src/$sid/$submitted_file" target="_blank">$submitted_file</a><BR/> \n);
				}
			}
		}		
		print qq(\t </td> \n);
		
		#end table row entry
        	print qq(</tr> \n);
            }
	}	
	close FILE;
	
	print "</table></div>";

	print qq(<p align=center><a class=button href="#" style="width:90" onclick="javascript:checkAll()" ><span>Select All</span></a></br>);
	print qq(<a class=button href="#" style="width:92" onclick="javascript:uncheckAll()" ><span>Deselect All</span></a></p>);
#	print qq(<a href="javascript:checkAll()" >Select All</a><br/>);
#	print qq(<a href="javascript:uncheckAll()" >Deselect All</a><br/>);
	
	print qq(<p align=center><a class=button style="width:150pt" href="#" onclick="document.selectSID.submit()" ><span>Next (select test cases)</span></a></p>);
	print "</form>";
	
	# generate list of students to grade (using user input)
	
	print "</body></html>";
}

sub find_attribute{
	my ($source_file, $attr, $value) = @_;
	my $mytree = HTML::TreeBuilder->new;
	$mytree->parse_file("$source_file");
	my @name_tags = $mytree->find_by_attribute($attr, $value);

	unless(@name_tags){
#		print "<p>Please update the solution: $source_file</p>";
#		print "<p>A tag with attribute $attr=$value NOT found. </p>";
#		print "<p>The &lt;input&gt; tag which takes the calculator input must have ";
#		print " the attribute $attr=$value</p>";
#		exit;
		return 0;
	}

	$mytree->delete;	
	return 1;
}
