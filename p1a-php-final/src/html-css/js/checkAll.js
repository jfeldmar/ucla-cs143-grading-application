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
