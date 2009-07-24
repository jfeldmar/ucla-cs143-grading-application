function ShowHideSection(section, button) {
  var tbl = document.getElementById(section);
  tbl.style.display = (button.value == 'Hide') ? 'none' : 'block';
  button.value = (button.value == 'Hide') ? 'Show' : 'Hide';
}

function editCell (cell, type, sid) {
	var input;
	if (type == "number")
		input = prompt("Please enter your text", "1");
	else
		input = prompt("Please enter your text", "Correct");
	
	if (input != null){
		if (type == "number")
		{
			if (!is_numeric(input))
				alert("Error: Please enter a number greater than 0");
			else{
				cell.innerHTML = input;
				update_total_score(sid);
			}
		}else if (type == "text"){
			cell.innerHTML = input;
			update_notes(sid);
		}else{
			alert("Error: Invalid editCell Input (not number or text)");
		}
	}
}

function is_numeric(input){
	return !isNaN(input) && (input >= 0);
}

function update_total_score(tablediv){
	var t =  document.getElementById(tablediv).getElementsByTagName("table")[0];
	var scores = t.getElementsByClassName("qscore");
	var sum = 0;
	for (i = 0; i < scores.length; i++)
	{
		sum += eval(scores[i].innerHTML);
	}
	t.getElementsByClassName("tscore")[0].innerHTML = sum;
}

function update_notes(tablediv){
	var t =  document.getElementById(tablediv).getElementsByTagName("table")[0];
	var scores = t.getElementsByClassName("qnotes");
	var notes_text = "Notes: ";
	for (i = 0; i < scores.length; i++)
	{
		if (i == 0)
			notes_text += " " + scores[i].innerHTML;
		else
			notes_text += "; " + scores[i].innerHTML;
	}
	t.getElementsByClassName("tnotes")[0].innerHTML = notes_text;
}

function update_totals()
{
	var subs = document.getElementsByClassName("submissions");
	for (var i = 0; i < subs.length; i++)
	{
		update_total_score(subs[i].id);
		update_notes(subs[i].id);
	}
}

// global javascript variables
// store the list of files in the pop-up window and the sid of the current student
var pop_up_text = "";
var cur_sid = "";
var phpFileToGrade = new Object;

// when the user wants to grade a different file submitted by a student
// a pop-up window comes up which gives a choice of all the files the student has submitted
// once the user click on one of the files, that file is plugged in as the source
// used to run all the test cases (shown in the student's table)
function ChooseFilePopUp(url, sid)
{
	pop_up_text = "";
	cur_sid = sid;
	// create code for the list of files the current student has submitted
	// when the user clicks on one, the pop-up is closed and that file is selected
	var arr = eval("files_"+sid);
	for (i in arr)
	{
		pop_up_text += '<input value=\"' + arr[i] + '\" type=button onclick=\"window.opener.close_window(window, this.value);\">';
		pop_up_text += '<a href="../submissions/'+sid + "/" + arr[i] + '" target="_blank"/>Preview File</a>';
		pop_up_text += "<br/>";
	}
		
	var strWindowFeatures = "modal=off,status=no,width=250,height=250,location=no,chrome=yes,centerscreen=yes";
	
	// close pop-up window if it's open
	if (typeof choose_file_window != 'undefined')
		choose_file_window.close();
	
	// open pop-up window with list of the student's submitted files
	var choose_file_window = window.open(url, "ChooseFile",strWindowFeatures);
	if (window.focus) choose_file_window.focus();
}

//Called when pop-up with choice of submitted files submits chosen file
function close_window(w, fname)
{
	//close pop-up
	window.focus();
	if (typeof w != 'undefined')
		w.close();
	//save name of currently graded file
	phpFileToGrade.cur_sid = fname;
	//reset the query and result links in table to point to selected file
	update_links(cur_sid, fname);
}

// given a student's SID and a new php-file-name to grade, update all the links
// in the student's table to point to the correct file
// also update results and points columns
function update_links(sid, nfile)
{
	// get the rows of the table for the given student (in div tag with id=SID)
	var rows = document.getElementById(sid).getElementsByTagName('tr');
	var temp;
	
	// queries start on the second row and end on the prelast row
	// therefore start at index 1 and end at index rows.length-2
	for (var r = 1; r < rows.length - 1; r++)
	{
		var php_link = rows[r].getElementsByClassName('php_editable')[0];
		var result_cell = rows[r].getElementsByClassName('phpresult')[0];
		var sample_cell = rows[r].getElementsByClassName('sampleresult')[0];
		var score_cell = rows[r].getElementsByClassName('qscore')[0];

		var re_str = "\\/" + sid + "\\/.*\\?";
		var RE = new RegExp(re_str, "i");

		// update link to student's submission appended with current query (in row r)
		php_link.href = php_link.href.replace(RE, "/" + sid + "/"+ nfile +"?");

		// get current file link from <td> id attribute
		// replace target file name
		// update id of tag to reflect new target
		// get new result using new href target
		// update contents of cell with result (using get_result function)
		temp = result_cell.id.replace(RE, "/" + sid + "/"+ nfile +"?");
		result_cell.id = temp;
		result_cell.innerHTML = get_result(temp);
		
		// update score/score total
		// note: the result in the result_cell is inside a <div> tag
		score_cell.innerHTML = recommend_score(sample_cell.innerHTML, result_cell.getElementsByTagName('div')[0].innerHTML);
	}
	
	
	// only edit once (link to graded file)
	var filelink =  document.getElementById(sid).getElementsByClassName('link_editable')[0];
	// the end of the link is sid/php_filename.php
	// replace php_filename.php with nfile (the new file to be graded)
	var re_str = sid + "\\/.*";
	var RE = new RegExp(re_str, "i");
	filelink.href = filelink.href.replace(RE, sid + "/" +nfile);
	filelink.innerHTML = nfile;
	
	//update total score
	update_total_score(sid);
}

// on update of file to grade, the results column must be updated
// this function uses AJAX to access the new file's result for given query,
// extracts the resulting value, and plugs it in the correct location in the student's table
function get_result(link)
{
	var xhr; 
	var err = "Error";
	var return_string = "";
	try {  xhr = new ActiveXObject('Msxml2.XMLHTTP');   }
	catch (e) 
	{
	    try {   xhr = new ActiveXObject('Microsoft.XMLHTTP');    }
	    catch (e2) 
	    {
              try {  xhr = new XMLHttpRequest();     }
              catch (e3) {  xhr = false;   }
	    }
	 }

	// get page html synchronously
       xhr.open("GET", link,  false); 
       xhr.send(null); 

       if(xhr.status  == 200) {
       		// attach returned html to a temporary div in the current document
	   var robj	= xhr.responseText;
	   var tempDIV = document.createElement("div");
	   document.body.appendChild(tempDIV);
	   tempDIV.innerHTML = xhr.responseText;

		// extract query result from html by finding the tag whose id='result'
	   var tag = document.getElementById('result');
	   if ( tag != null && is_numeric(tag.innerHTML) ){	// if value exists
		   var rv = tag.innerHTML;
		   return_string = "<div onmouseover=\"Tip('Answer Correct')\" onmouseout=\"UnTip()\">" + rv + "</div>";
		   // remove the temporary div tag
		   document.body.removeChild(tempDIV);
		   
		   // return the inner contents of found tag
		   return return_string;
		   }
	   else{	// => no 'result' id found
		   // remove the temporary div tag
		   document.body.removeChild(tempDIV);
		   return_string = "<div onmouseover=\"Tip('Unable to find \\'result\\' tag')\" onmouseout=\"UnTip()\">Error</div>";
		   // return error value with a bubble message
		   return return_string;
		   }

       }
       else {
		   // return error value with a bubble message
		   return_string = "<div onmouseover=\"Tip('Unable to open "+link+"')\" onmouseout=\"UnTip()\">Error</div>";
		   return return_string;
		   }

}

// Shows the tables for all submissions
// 1. gets all buttons from document
// 2. for buttons whose value is 'Show' (their sections are hidden), 
//	the submission is shown, and button value changed to 'Hide'
function show_all(show)
{
	var tbl = document.getElementsByTagName('input');

	for (var j in tbl)
	{
		if (tbl[j].value == show)
		{
			var section = document.getElementById(tbl[j].className);
			section.style.display = 'block';
			tbl[j].value = 'Hide';
		}
	}
}

// Hides the tables for all submissions
// 1. gets all buttons from document
// 2. for buttons whose value is 'Hide' (their sections are shown), 
//	the submission is hidden, and button value changed to 'Show'
function hide_all(hide)
{
	var tbl = document.getElementsByTagName('input');

	for (var j in tbl)
	{
		if (tbl[j].value == hide)
		{
			var section = document.getElementById(tbl[j].className);
			section.style.display = 'none';
			tbl[j].value = 'Show';
		}
	}
}

// given expected query result and the submission's result
// recommend score between 0 and 1 where 1 is 100% correct
function recommend_score(expected, received)
{
	if (is_numeric(expected) && is_numeric(received))
	{
		if (parseFloat(expected) == parseFloat(received))
			return "1";
		else
			return "0.5";
	}else { return "0"; }
}
