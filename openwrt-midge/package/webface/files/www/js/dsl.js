function SetupDSLParams() {
	while($('rate').length > 0) {
		$('rate').remove($('rate').length - 1);
	}

	max = 96;
	switch($('code').value) {
		case 'tcpam4': max=10; break;
		case 'tcpam8': max=20; break;
		case 'tcpam16': max=50; break;
		case 'tcpam32': max=96; break;
	};


	for (i=1; i < max; i ++) {
		var e=document.createElement('option');
		e.text=(i*64).toString();
		e.value=(i*64).toString();
		try {
			$('rate').add(e);
		} catch(ex) {
			$('rate').add(e,null);
		}
	}


}

function OnChangeDSLCode() {
	SetupDSLParams();
}

