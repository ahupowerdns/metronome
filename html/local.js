var updateEverything = function(){}
$(document).ready(function() {
    $.ajaxSetup({ cache: false });
    var comconfig = { url: "http://127.0.0.1:8000/", beginTime: -3*3600 };
    
    function showAll()
    {
	var servername=$("#server").val();
	if(servername=='')
	    return;

	var config1 = { items: [ 
            { name: "pdns."+servername+".servfail-answers", legend: "Servfail answers/s"},
            { name: "pdns."+servername+".questions", legend: "Questions/s" }, 
            { name: "pdns."+servername+".all-outqueries", legend: "All outqueries/s"}
        ]};
	
	
	var config2 = { renderer: 'stack', items: [ 
            { name: "pdns."+servername+".answers-slow", legend: "Slow answers/s"},
	    { name: "pdns."+servername+".packetcache-hits", legend: "0ms answers/s"},
            { name: "pdns."+servername+".answers0-1", legend: "<1 ms answers/s"},
            { name: "pdns."+servername+".answers1-10", legend: "1-10ms answers/s"},
            { name: "pdns."+servername+".answers10-100", legend: "10-100ms answers/s"},
            { name: "pdns."+servername+".answers100-1000", legend: "100-1000ms answers/s"}
	    
	]};
	
	var config2a = { renderer: 'stack', items: [ 
            { name: "pdns."+servername+".servfail-answers", legend: "SERVFAIL answers/s"},
            { name: "pdns."+servername+".nxdomain-answers", legend: "NXDOMAIN answers/s"},
            { name: "pdns."+servername+".noerror-answers", legend: "Normal answers/s"}
	    
	    
	]};
	
	
	var config3 ={ renderer: "stack", items: [ 
            { metrics: ["pdns."+servername+".user-msec"], legend: "User CPU%",
	      formula: function(r,d) { return d[0]/10.0; }
	    }, 
            { metrics: ["pdns."+servername+".sys-msec"], legend: "System CPU%",
	      formula: function(r,d) { return d[0]/10.0; }
	    }]};
	
	var config3a = { items: [ 
            { name: "pdns."+servername+".throttled-out", legend: "Throttled/s" }, 
            { name: "pdns."+servername+".resource-limits", legend: "Resource limited/s"}]};

	var config3b ={ items: [
            { name: "pdns."+servername+".concurrent-queries", legend: "Concurrent queries", kind: "gauge"}
        ]};

	
	var config4 = { items: [ 
            { 
		metrics: ["pdns."+servername+".packetcache-hits","pdns."+servername+".packetcache-misses"], 
		legend: "% packetcache hitrate", 
		formula: percentalizer
	    }]};    

	var config5 = { items: [ 
            { 
		metrics: ["pdns."+servername+".cache-hits","pdns."+servername+".cache-misses"], 
		legend: "% cache hitrate", 
		formula: percentalizer
	    }]};    


	var config6 ={ renderer: "stack", items: [
            { name: "pdns."+servername+".cache-entries", legend: "Cache entries", kind: "gauge"}, 
            { name: "pdns."+servername+".packetcache-entries", legend: "Packetcache entries", kind: "gauge"}]};

/*
	var config7 = { items: [ 
            { name: "pdns."+servername+".auth.udp-queries", legend: "Questions/s" }, 
            { name: "pdns."+servername+".auth.udp-answers", legend: "All outqueries/s"}]};


	var config8 = { items: [ 
            { 
		metrics: ["pdns."+servername+".auth.packetcache-hit","pdns."+servername+".auth.packetcache-miss"], 
		legend: "% packet cache hitrate", 
		formula: percentalizer
	    }]};    

	var config9 = { items: [ 
            { 
		metrics: ["pdns."+servername+".auth.query-cache-hit","pdns."+servername+".auth.query-cache-miss"], 
		legend: "% query cache hitrate", 
		formula: percentalizer
 	    }]};    

*/
	showStuff(comconfig, config1, "#hier1");
	showStuff(comconfig, config2, "#hier2");
	showStuff(comconfig, config2a, "#hier2a");	
	showStuff(comconfig, config3, "#hier3");
	showStuff(comconfig, config3a, "#hier3a");	
	showStuff(comconfig, config3b, "#hier3b");	
	showStuff(comconfig, config4, "#hier4");
	showStuff(comconfig, config5, "#hier5");
	showStuff(comconfig, config6, "#hier6");
/*
	showStuff(comconfig, config7, "#hier7");
	showStuff(comconfig, config8, "#hier8");
	showStuff(comconfig, config9, "#hier9");	
*/
    }
    updateEverything = function() { comconfig.beginTime = parseInt($("#duration").val()); ; showAll(); };    
  
    getServers(comconfig, function(servers) { 
	var ret="";
	$.each(servers, function(a,b) {
	    ret+= "<option value='"+b+"'>"+b+"</option>";
	});
	$("#server").html(ret);
	updateEverything();
	setInterval(updateEverything, 5000);    
    });
 
});
