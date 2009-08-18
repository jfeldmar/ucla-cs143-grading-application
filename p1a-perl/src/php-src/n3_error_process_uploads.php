<?php

###################################################
## extract provided zip file, confirm correct file hierarchy
## upload/link to sample calculator solution, confirm functionality
## display/link to all students' submissions
###################################################
	$submitted_php_extension = "php";
	$submitted_txt_extension = "txt";

	$safe_filename_characters = "a-zA-Z0-9_.-";
	$upload_dir = "../file-uploads";
	$zip_filename = "project1A-Submissions.tar";
	$php_filename = "sampleCalculator.php";
	$submissions_directory = "../submissions";
	$temp_directory = "../temp";
	$html_images_dir = "../html-css/images/";
	$editable_src = "editable_src";
	$submissions_csv_file = "../submissions/submission.csv";


echo header('Content-type: text/html');
?>

<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
	<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />
</head>
<body>
<script type="text/javascript" src="../html-css/js/wz_tooltip.js"></script>
<script type="text/javascript" src="../html-css/js/checkAll.js"></script>
<h1>CS143 - Project 1A Grading Application</h1>


<?php

# If sample Calculator PHP solution was uploaded by user
# Display a link to it
if (file_exists("$upload_dir/$php_filename"))
{
	###################################################
	## Sample Submission Testing
	## 1. Result has correct name tag
	## 2. Link to simple test case
	###################################################

	echo "<p>Please Test:&nbsp;&nbsp;&nbsp;";
	echo "<a href=\"$upload_dir/$php_filename\" target=_blank>Your Sample Submission</a> * \n";
	echo "<a href=\"$upload_dir/$php_filename?expr=2%2B3\" target=_blank>Simple Test Case: 2+3</a>\n";

	if (find_attribute("$upload_dir/$php_filename", "name", "expr")){
	     echo "&nbsp;<img src=\"$html_images_dir/greenCheck.gif\" onmouseover=\"Tip(\OK:\" onmouseout=\"UnTip()\"/> </p>\n";
	}else{
	     echo "&nbsp;<img src=\"$html_images_dir/redX.gif\" class=error
	     onmouseover=\"Tip('ERROR: Attribute name=expr NOT found; unable to process file\')\" onmouseout=\"UnTip()\" /></p>\n";
	}
}

###################################################
## Tar file of submissions
## 1. Display files generated (missing files, missing ID tag,etc.)
## 2. copy each student's submitted files into a new subdirectory
##	to allow for easier preview and editing of submitted source code
## 3. Allow user to generate list of programs to grade
###################################################

/*
# delete editable_src directory in temporary directory (clean up)
if (-d "$temp_directory/$editable_src"){
	rmtree("$temp_directory/$editable_src",0, 0 ) or die("Cannot remove contents of $temp_directory/$editable_src directory");		
}
mkdir("$temp_directory/$editable_src") or die "Cannot create $temp_directory/$editable_src directory";
*/

# 1. Display files generated (missing files, missing ID tag,etc.)

# import submissions.csv
$FILE = fopen($submissions_csv_file, 'r') or die("Can't open $submissions_csv_file file");

echo '<form name=selectSID method="POST" action="../php-src/n4_choose_test_cases.php">';
echo "\n";

# form submit button
echo '<p align=center><a class=button style="width:150pt" href="#" onclick="document.selectSID.submit()" ><span>Next (select test cases)</span></a></p>';
echo "\n";

# select all button
echo '<p align=center><a class=button href="#" style="width:90" onclick="javascript:checkAll()" ><span>Select All</span></a></br>';
echo "\n";
# deselect all button
echo '<a class=button href="#" style="width:92" onclick="javascript:uncheckAll()" ><span>Deselect All</span></a></p>';
echo "\n";

# note to user
echo "<div align=center><font size=2>(some students have multiple submissions)</font></div>\n";

echo "<div align=center><table>\n";
echo '<tr><th></th><th>SID</th><th>Name</th><th>Source Files</th><th>Interact with Code</th></tr>';


while($line = fgetcsv($FILE, 0, ','))
{
    //@fields = $csv_submissions->fields();
    $sid = $line[0];
    $name = $line[1];
    #begin table row/checkbox
    echo "<tr id=$sid> \n";		#prints id=SID for <tr> tag "id"
    echo "\t <td><input type=checkbox name='check' value=$sid></td> \n";
    echo "\t <td>$sid</td> \n";		#prints SID
    echo "\t <td>$name</td> \n";		#prints Name

    #print link to file source (add red X if no attribute found)
    $DIR = opendir("$submissions_directory/$sid") or die("Can't open submission directory for $sid:$name");
    $files = array();
    while ( $f = readdir($DIR))
    {
    	if ($f != "." && $f != "..")
		array_push($files, $f);
    }
    closedir($DIR);
    
    ## 2.1 create for each student's submitted files a new (editable_src) directory in the temporary directory
    ##	to allow for easier preview and editing of submitted source code
#		if (-d "$temp_directory/$editable_src/$sid"){
#			rmtree("$temp_directory/$editable_src/$sid",0, 0 ) or die("Cannot remove contents of $temp_directory/$editable_src/$sid directory");		
#		}
#		mkdir("$temp_directory/$editable_src/$sid") or die "Cannot create $temp_directory/$editable_src/$sid directory";


    # list links to submitted files
    echo "\t <td> \n";

    foreach ($files as $submitted_file){
	    #print only files, not directories
	    if ( file_exists("$submissions_directory/$sid/$submitted_file") ){
		 if (preg_match("/.$submitted_php_extension$/", $submitted_file)){
			 $error;
			 $tooltip;
			 if ( find_attribute("$submissions_directory/$sid/$submitted_file", "name", "expr") ){
			    $error = 0;
			    $tooltip = 'onmouseover="Tip(\'OK: Attribute name=expr found\')" onmouseout="UnTip()"';
			 }else{
			    $error = 1;
			    $tooltip = ' class=error onmouseover="Tip(\'ERROR: Attribute name=expr NOT found; unable to process file\')" onmouseout="UnTip()"';
			 }

			 echo "<a href=\"$submissions_directory/$sid/$submitted_file\" target=\"_blank\" $tooltip >$submitted_file</a> \n";

			 if ($error){
			      echo "<img src=\"$html_images_dir/redX.gif\" $tooltip /><BR/>";
			 }else{
			      echo "<img src=\"$html_images_dir/greenCheck.gif\" $tooltip/> <BR/>";
			 }
		 }else if (preg_match("/.$submitted_txt_extension$/", $submitted_file)){
			 echo "<a href=\"$submissions_directory/$sid/$submitted_file\" target=\"_blank\" >$submitted_file</a> \n";
			 echo " </BR>";
		 }
	    }
    }
    echo "\t </td> \n";

    #print link to Calculator solution (add red X if no attribute found)
    # add green check if required attribute value found
    echo "\t <td> \n";
    foreach ($files as $submitted_file){
	    #print only files, not directories
	    if( file_exists("$submissions_directory/$sid/$submitted_file")){
		    $SUBFILE = fopen( "$submissions_directory/$sid/$submitted_file", 'r') or die("Can't open $submitted_file for $sid:$name");
		    $lines = fread($SUBFILE, filesize("$submissions_directory/$sid/$submitted_file"));
		    fclose($SUBFILE);

		    ## 3.2 copy each student's submitted files into a new (editable_src) subdirectory
		    ##	to allow for easier preview and editing of submitted source code
#			open CFILE, ">$temp_directory/$editable_src/$sid/$submitted_file" or die "unable to create file $submissions_directory/$sid/$editable_src/$submitted_file";
#			# reformat text to display php code literally (without execution)
#			if ($submitted_file =~ m/.$submitted_php_extension$/){
#				$source = encode_entities($source);
#				$source = "<pre>".$source."<pre>";
#			}
#			echo CFILE $source;
#			close CFILE;

		    if (preg_match( "/.$submitted_php_extension$/", $submitted_file)){
			    echo "<a href=\"$temp_directory/$editable_src/$sid/$submitted_file\" target=\"_blank\">Source: $submitted_file</a><BR/> \n";
		    }else if (preg_match( "/.$submitted_txt_extension$/", $submitted_file)){
			    echo "<a href=\"$temp_directory/$editable_src/$sid/$submitted_file\" target=\"_blank\">$submitted_file</a><BR/> \n";
		    }
	    }
    }		

    echo "\t </td> \n";

    #end table row entry
    echo "</tr> \n";
}	
fclose($FILE);

?>

</table></div>

<p align=center><a class=button href="#" style="width:90" onclick="javascript:checkAll()" ><span>Select All</span></a></br>

<a class=button href="#" style="width:92" onclick="javascript:uncheckAll()" ><span>Deselect All</span></a></p>

<p align=center><a class=button style="width:150pt" href="#" onclick="document.selectSID.submit()" ><span>Next (select test cases)</span></a></p>

</form>
</body></html>

<?php

function find_attribute($source_file, $attr, $value){
	$doc = new DomDocument();
	@$doc->loadHtmlFile("$source_file");
	$xpath = new DOMXPath($doc);
	
	// find at least one $attr=$value attribute/name pair
	$expr = '//*[@' . $attr . '="' . $value . '"]';
	$attributes = $xpath->evaluate($expr);

	if(empty($attributes)){
	#		echo "<p>Please update the solution: $source_file</p>";
	#		echo "<p>A tag with attribute $attr=$value NOT found. </p>";
	#		echo "<p>The &lt;input&gt; tag which takes the calculator input must have ";
	#		echo " the attribute $attr=$value</p>";
	#		exit;
		return 0;
	}

	return 1;
}

?>
