"use strict";

var updateEverything = function(){}
$(document).ready(function() {
    $.ajaxSetup({ cache: false });
    
    var comconfig = { url: "http://xs.powerdns.com:8000/", beginTime: -3*3600, datapoints: 100 };
    var recursordivconfig=[], authdivconfig=[];

    $(window).bind('popstate',  
		   function(event) {
		       if(event.originalEvent.state != undefined) {
			   $("#server").val(event.originalEvent.state.server);
			   $("#duration").val(event.originalEvent.state.beginTime);
			   var ret = configAll();
			   recursordivconfig=ret[0];
			   authdivconfig=ret[1];

			   updateEverything();
		       }
		   });

    function configAll()
    {
	var servername=$("#server").val();
	if(servername=='')
	    return;

	var config1 = { where: 'recursor1', items: [ 
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
	
	var config2b = { items: [ 
            { name: "pdns."+servername+".tcp-questions", legend: "TCP/IP questions/s"},
            { name: "pdns."+servername+".tcp-outqueries", legend: "TCP/IP outqueries/s"},
            { name: "pdns."+servername+".tcp-client-overflow", legend: "TCP/IP overflows/s"}	    
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


	var config7 = { items: [ 
            { name: "pdns."+servername+".udp-queries", legend: "Questions/s" }, 
            { name: "pdns."+servername+".udp-answers", legend: "Answers/s"}]};

	var config7a = { items: [ 
            { name: "pdns."+servername+".tcp-queries", legend: "TCP Questions/s" }, 
            { name: "pdns."+servername+".tcp-answers", legend: "TCP Answers/s"}]};

	var config7b = { items: [ 
            { name: "pdns."+servername+".qsize-q", legend: "DB Queue", kind: "gauge"}
            ]};


	var config8 = { items: [ 
            { 
		metrics: ["pdns."+servername+".packetcache-hit","pdns."+servername+".packetcache-miss"], 
		legend: "% packet cache hitrate", 
		formula: percentalizer
	    }]};    

	var config9 = { items: [ 
            { 
		metrics: ["pdns."+servername+".query-cache-hit","pdns."+servername+".query-cache-miss"], 
		legend: "% query cache hitrate", 
		formula: percentalizer
 	    }]};    

	var config10 = { items: [ 
		{name: "pdns."+servername+".query-cache-miss", legend: "Database queries/s"}
	]};
		
 	       
	var recconfigs=[config1, config2, config2a, config2b, config3, config3a, config3b, config4, config5, config6];
	var authconfigs=[config7, config7a, config7b, config8, config9, config10];
	
	var recursordivconfigs=setupMetronomeHTML("#recursor", recconfigs);
	var authdivconfigs=setupMetronomeHTML("#auth", authconfigs);

	return [recursordivconfigs, authdivconfigs];
    }

    function showAll(recursordivconfigs, authdivconfigs)
    {
	var servername=$("#server").val();
	if(servername=='')
	    return;

	if(servername.split('.')[1]=="recursor") { 
	    for(var a in recursordivconfigs) {
		showStuff(comconfig, recursordivconfigs[a][0], recursordivconfigs[a][1]);
	    }
	    $("#auth").hide();
	    $("#recursor").show();
	}
	else if(servername.split('.')[1]=="auth") { 
	    for(var a in authdivconfigs) {
		showStuff(comconfig, authdivconfigs[a][0], authdivconfigs[a][1]);
	    }
	    $("#auth").show();
	    $("#recursor").hide();
	}

    }
    
    var interval;     

    window.updateEverything = function() { 
	comconfig.beginTime = parseInt($("#duration").val());
	
	if(interval != undefined)
	    clearInterval(interval);
	interval = setInterval(updateEverything, -comconfig.beginTime*1000/comconfig.datapoints);    
	// console.log("New interval: ", -comconfig.beginTime*1000/comconfig.datapoints);
	showAll(recursordivconfig, authdivconfig); 
    };    

    window.updateFromForm = function() {
	var ret = configAll();
	recursordivconfig=ret[0];
	authdivconfig=ret[1];

	var stateObj = { server: $("#server").val(), beginTime: parseInt($("#duration").val()) };
	history.pushState(stateObj, "Metronome", "?server="+stateObj.server+"&beginTime="+stateObj.beginTime);
	updateEverything();
    }


    getServers(comconfig, function(servers) { 

	var ret="";
	$.each(servers, function(a,b) {
	    ret+= "<option value='"+b+"'>"+b+"</option>";
	});
	$("#server").html(ret);

	if($.url().param('server') != undefined) {
	    $("#server").val($.url().param('server'));
	}
	if($.url().param('beginTime') != undefined)
	    $("#duration").val($.url().param('beginTime'));
	
	var stateObj = { server: $("#server").val(), beginTime: parseInt($("#duration").val()) };
	history.replaceState(stateObj, "Metronome", "?server="+stateObj.server+"&beginTime="+stateObj.beginTime);
	var configs = configAll();
	recursordivconfig=configs[0];
	authdivconfig=configs[1];

	updateEverything();
    });
 
});
