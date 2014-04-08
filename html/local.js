var updateEverything = function(){}
$(document).ready(function() {
    $.ajaxSetup({ cache: false });
    
    var comconfig = { url: "http://xs.powerdns.com:8000/", beginTime: -3*3600 };

    $(window).bind('popstate',  
		   function(event) {
		       if(event.originalEvent.state != undefined) {
			   $("#server").val(event.originalEvent.state.server);
			   $("#duration").val(event.originalEvent.state.beginTime);
			   updateEverything();
		       }
		   });

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
		
 	       

	
	if(servername.split('.')[1]=="recursor") { 
	    showStuff(comconfig, config1, "#hier1");
	    showStuff(comconfig, config2, "#hier2");
	    showStuff(comconfig, config2a, "#hier2a");	
	    showStuff(comconfig, config3, "#hier3");
	    showStuff(comconfig, config3a, "#hier3a");	
	    showStuff(comconfig, config3b, "#hier3b");	
	    showStuff(comconfig, config4, "#hier4");
	    showStuff(comconfig, config5, "#hier5");
	    showStuff(comconfig, config6, "#hier6");
	    $("#auth").hide();
	    $("#recursor").show();
	}
	if(servername.split('.')[1]=="auth") {
	    showStuff(comconfig, config7, "#hier7");
	    showStuff(comconfig, config7a, "#hier7a");	    
	    showStuff(comconfig, config7b, "#hier7b");	    	    
	    showStuff(comconfig, config8, "#hier8");
	    showStuff(comconfig, config9, "#hier9");
	    showStuff(comconfig, config10, "#hier10");
	    $("#auth").show();
	    $("#recursor").hide();
	}	
    }
   
    updateEverything = function() { 
	comconfig.beginTime = parseInt($("#duration").val());
	showAll(); 
    };    

    updateFromForm = function() {
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

	updateEverything();
	setInterval(updateEverything, 5000);    
    });
 
});
