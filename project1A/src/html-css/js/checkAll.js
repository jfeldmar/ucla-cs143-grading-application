function checkAll()
{
	if(!document.forms)
		return;
	var obj = document.forms[0].elements;

	for(var j = 0; j < obj.length; j++)
	{
		if(obj[j].type == 'checkbox')
			obj[j].checked = true;
	}
}

function uncheckAll()
{
	if(!document.forms)
		return;
	var obj = document.forms[0].elements;

	for(var j = 0; j < obj.length; j++)
	{
		if(obj[j].type == 'checkbox')
			obj[j].checked = false;
	}
}

function submit_checked(form)
{
	if(!document.forms)
		return;
	var obj = document.forms[0].elements;
	
	var counter = 0;

	for(var j = 0; j < obj.length; j++)
	{
		if(obj[j].type == 'checkbox')
			if (obj[j].checked == true)
				counter += 1;
	}
	
	if (counter > 0)
		form.submit();
	else
		alert("Please select at least one submission to grade");
}
