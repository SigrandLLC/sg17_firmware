
function freeList(l){
    
    while ( l.length > 0 ){
		//	 alert("remove " + l.options[l.length-1].value);
        try{
            l.options.remove(l.length-1);
        }catch(ex){
			//	     alert("Cannot remove elem l.length-1");
            l.options[l.length-1] = null;
    	    l.length--;
		};
    };
};
	

function rate_list(l,st,end,step,cur){
    freeList(l);
	ind=0;
	ind_res=0;

    for(i=st;i<=end;i+=step){
    	el = new Option;
    	el.value = i;
    	el.text = i;
		el.selected = 0;
		if( i == cur ){
    	    el.selected = 1;
			ind_res = ind;
		};
		try{
			l.options.add(el);
		} catch(ex){
			l.options.add(el,null);
		};
		ind++;
    };
	
	l.selectedIndex = ind_res;
};


function OnChangeSG16Code(){

	//alert("Start SG16 change");
    TCPAM = new Array();
	TCPAM[0] = new Array();
	TCPAM[0][0] = "tcpam4";
	TCPAM[0][1] = "TCPAM4";		
	TCPAM[1] = new Array();	
	TCPAM[1][0] = "tcpam8";
	TCPAM[1][1] = "TCPAM8";		
	TCPAM[2] = new Array();	
	TCPAM[2][0] = "tcpam16";
	TCPAM[2][1] = "TCPAM16";		
	TCPAM[3] = new Array();	
	TCPAM[3][0] = "tcpam32";
	TCPAM[3][1] = "TCPAM32";		

    preact = $('cfg').options[$('cfg').selectedIndex].value;
    annex = $('annex').options[$('annex').selectedIndex].value;
    mode = $('mode').options[$('mode').selectedIndex].value;
    tcpam = $('code').options[$('code').selectedIndex].value;

	ind = $('rate').selectedIndex;
	if( ind < 0 || ind==undefined ){
		ind = 0;
		rate = 64
	}else{
		rate = $('rate').options[ind].value;
	}


    if( preact == "preact" && annex == "F"){
		// TCPAM 16/32
		// Rate: 192 - 5696 for master, automatic for slave
		freeList($('code'));
		if( mode == "slave" ){
			var el = new Option;
			el.value = TCPAM[3][0];
			el.text = TCPAM[3][1];
			$('code').options.add(el);
			$('code').disabled = 1;
			freeList($('rate'));
			$('rate').options[0] = new Option("automatic");
			$('rate').disabled = 1;
		} else {
			$('code').disabled = 0;
			for(i=3;i>=2;i--){
				var el = new Option;
				el.value = TCPAM[i][0];
				el.text = TCPAM[i][1];
				if( TCPAM[i][0] == tcpam ){
					el.selected = 1;
				};
				$('code').options.add(el);
			};
			$('rate').disabled = 0;
			tcpam = $('code').options[$('code').selectedIndex].value;
			if( tcpam == "tcpam16" )
				rate_list($('rate'),192,2304,64,rate);
			else
				rate_list($('rate'),192,5696,64,rate);
		};
    }else if( preact == "preact"){
		freeList($('code'));
		var el = new Option;
		el.value = TCPAM[2][0];
		el.text = TCPAM[2][1];
		$('code').options.add(el);
		$('code').disabled = 1;

		if( mode == "slave" ){
			freeList($('rate'));
			$('rate').options[0] = new Option("automatic");
			$('rate').disabled = 1;
		} else {
			$('rate').disabled = 0;
			rate_list($('rate'),192,2304,64,rate);	
		};
    } else {
		freeList($('code'));
		$('code').disabled = 0;
		for(i=3;i>=0;i--){
			var el = new Option;
			el.value = TCPAM[i][0];
			el.text = TCPAM[i][1];
			if( TCPAM[i][0] == tcpam ){
				//			alert("select " + tcpam);
				el.selected = 1;
			};
			$('code').options.add(el);
			if( TCPAM[i][0] == tcpam ){
				$('code').selectedIndex = 3-i;
			};
		};

		$('rate').disabled = 0;
		tcpam = $('code').options[$('code').selectedIndex].value;
		if( tcpam == "tcpam4" ){
			rate_list($('rate'),64,704,64,rate);
		} else if( tcpam == "tcpam8" ){
			rate_list($('rate'),192,1216,64,rate);
		} else  if( tcpam == "tcpam16" ) {
			rate_list($('rate'),192,3840,64,rate);
		} else  if( tcpam == "tcpam32" ){
			rate_list($('rate'),256,6016,64,rate);
		};
    };
};



function OnChangeSG17Code()
{
    TCPAM = new Array();
	TCPAM[0] = new Array();
	TCPAM[0][0] = "tcpam16";
	TCPAM[0][1] = "TCPAM16";		
	TCPAM[1] = new Array();	
	TCPAM[1][0] = "tcpam32";
	TCPAM[1][1] = "TCPAM32";		

    mode = $('mode').options[$('mode').selectedIndex].value;
    tcpam = $('code').options[$('code').selectedIndex].value;
	clkmode_ind = $('clkmode').selectedIndex;
	ind = $('rate').selectedIndex;
//	alert('ind=' + ind);
	if( ind < 0 ){
		ind = 0;
		rate = 192;
	}else{
		rate = $('rate').options[ind].value;
	}
//	alert('get rate = ' + rate + ' ind= ' + ind);

    if( mode == "slave" ){
		freeList($('rate'));
		$('rate').options[0] = new Option("automatic");
		$('rate').disabled = 1;
		freeList($('code'));
		$('code').options[0] = new Option("automatic");
		$('code').disabled = 1;
		freeList($('clkmode'));
		$('clkmode').options[0] = new Option("automatic");
		$('clkmode').disabled = 1;
    } else {
		freeList($('code'));
		$('code').disabled = 0;
		for(i=1;i>=0;i--){
			var el = new Option;
			el.value = TCPAM[i][0];
			el.text = TCPAM[i][1];
			if( TCPAM[i][0] == tcpam ){
				el.selected = 1;
			};
			$('code').options.add(el);
			if( TCPAM[i][0] == tcpam ){
				$('code').selectedIndex = 1-i;
			};
		};

		$('rate').disabled = 0;
		tcpam = $('code').options[$('code').selectedIndex].value;
		if( tcpam == "tcpam16" ) {
			rate_list($('rate'),192,3840,64,rate);
		} else if( tcpam == "tcpam32" ){
			rate_list($('rate'),768,5696,64,rate);
		};
		
		freeList($('clkmode'));
		$('clkmode').disabled = 0;
		$('clkmode').options[0] = new Option("plesio");
		$('clkmode').options[1] = new Option("sync");
		$('clkmode').selectedIndex=clkmode_ind;
    }
};

function eocRates(){

	for(var i=1;;i++){
		s = $('rate' + i);
		//alert('s = ' + s);
		if( s == null ){
			break;
		}
		ind = s.selectedIndex;
		//alert('ind = ' + ind);
		if( ind == undefined ){
			break;
		}

		if( ind < 0 ){
			ind = 0;
			rate = 192;
		}else{
			rate = s.options[ind].value;
		}
		
		rate_list(s,192,5696,64,rate);
	}

	s = $('rate');
	//alert('s = ' + s);
	if( s == null ){
		return;
	}
	ind = s.selectedIndex;
	//alert('ind = ' + ind);
	if( ind == undefined ){
		return;
	}
	if( ind < 0 ){
		ind = 0;
		rate = 192;
	}else{
		rate = s.options[ind].value;
	}
		
	rate_list(s,192,5696,64,rate);
};

function aaa(){
    alert("aaa func");
}
