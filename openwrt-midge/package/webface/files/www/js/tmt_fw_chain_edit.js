function OnChange_nat(field) {
	var value=field.value;
	natto=$("natto");
	if (natto) {
		if (value=="DNAT" || value=="SNAT") {
			alert("Onchange_nat(): " + field.value);
			natto.setAttribute("tmt:required", "true");
		} else 
			natto.setAttribute("tmt:required", "");

		var formNodes = document.getElementsByTagName("form");
		for(var i=0; i<formNodes.length; i++)
			formNodes[i].onsubmit = null;

		tmt_validatorInit();
	}
}
