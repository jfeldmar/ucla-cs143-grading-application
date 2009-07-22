
function get_result(refobj)
{
	var xhr; 
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

	xhr.onreadystatechange  = function()
	{ 
             if(xhr.readyState  == 4)
             {
        	  if(xhr.status  == 200) {
		      var robj	= xhr.responseText;
		      var tempDIV = document.createElement("div");
		      document.body.appendChild(tempDIV);
		      tempDIV.innerHTML = xhr.responseText;

		      var tag = document.getElementById('result');
		      
		      if (tag != null && is_numeric(tag.innerHTML))
			      refobj.value = tag.innerHTML;
		      else
			      refobj.value = "Error";
		      
		      document.body.removeChild(tempDIV);
		  }
        	  else 
			      refobj.value = "Error";
             }
	}; 

       xhr.open("GET", refobj.id,  true); 
       xhr.send(null); 

}
