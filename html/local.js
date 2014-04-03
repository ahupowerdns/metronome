$(document).ready(function() {
    $.ajaxSetup({ cache: false });
    
    var hostname='micro';
    var comconfig = { url: "http://127.0.0.1:8000/", beginTime: -3600 };

    var config1 = { items: [ 
        { name: "pdns."+hostname+".recursor.questions", legend: "Questions/s" }, 
        { name: "pdns."+hostname+".recursor.all-outqueries", legend: "All outqueries/s"}]};


    var config2 = { renderer: 'stack', items: [ 
        { name: "pdns."+hostname+".recursor.answers-slow", legend: "Slow answers/s"},
	{ name: "pdns."+hostname+".recursor.packetcache-hits", legend: "0ms answers/s"},
        { name: "pdns."+hostname+".recursor.answers0-1", legend: "<1 ms answers/s"},
        { name: "pdns."+hostname+".recursor.answers1-10", legend: "1-10ms answers/s"},
        { name: "pdns."+hostname+".recursor.answers10-100", legend: "10-100ms answers/s"},
        { name: "pdns."+hostname+".recursor.answers100-1000", legend: "100-1000ms answers/s"}

    ]};

    var config3 ={ renderer: "stack", items: [ 
        { metrics: ["pdns."+hostname+".recursor.user-msec"], legend: "User CPU%",
	  formula: function(r,d) { return d[0]/10.0; }
	}, 
        { metrics: ["pdns."+hostname+".recursor.sys-msec"], legend: "System CPU%",
	  formula: function(r,d) { return d[0]/10.0; }
	}]};

    var config3a = { items: [ 
        { name: "pdns."+hostname+".recursor.throttled-out", legend: "Throttled/s" }, 
        { name: "pdns."+hostname+".recursor.resource-limits", legend: "Resource limited/s"}]};


    
    var config4 = { items: [ 
        { 
	    metrics: ["pdns."+hostname+".recursor.packetcache-hits","pdns."+hostname+".recursor.packetcache-misses"], 
	    legend: "% packetcache hitrate", 
	    formula: percentalizer
	}]};    

    var config5 = { items: [ 
        { 
	    metrics: ["pdns."+hostname+".recursor.cache-hits","pdns."+hostname+".recursor.cache-misses"], 
	    legend: "% cache hitrate", 
	    formula: percentalizer
	}]};    


    var config6 ={ renderer: "stack", items: [
        { name: "pdns."+hostname+".recursor.cache-entries", legend: "Cache entries", kind: "gauge"}, 
        { name: "pdns."+hostname+".recursor.packetcache-entries", legend: "Packetcache entries", kind: "gauge"}]};

    var config7 = { items: [ 
        { name: "pdns."+hostname+".auth.udp-queries", legend: "Questions/s" }, 
        { name: "pdns."+hostname+".auth.udp-answers", legend: "All outqueries/s"}]};


    var config8 = { items: [ 
        { 
	    metrics: ["pdns."+hostname+".auth.packetcache-hit","pdns."+hostname+".auth.packetcache-miss"], 
	    legend: "% packet cache hitrate", 
	    formula: percentalizer
	}]};    

    var config9 = { items: [ 
        { 
	    metrics: ["pdns."+hostname+".auth.query-cache-hit","pdns."+hostname+".auth.query-cache-miss"], 
	    legend: "% query cache hitrate", 
	    formula: percentalizer
 	}]};    

    function showAll()
    {
	showStuff(comconfig, config1, "#hier1");
	showStuff(comconfig, config2, "#hier2");
	showStuff(comconfig, config3, "#hier3");
	showStuff(comconfig, config3a, "#hier3a");	
	showStuff(comconfig, config4, "#hier4");
	showStuff(comconfig, config5, "#hier5");
	showStuff(comconfig, config6, "#hier6");
	showStuff(comconfig, config7, "#hier7");
	showStuff(comconfig, config8, "#hier8");
	showStuff(comconfig, config9, "#hier9");	
    }
    showAll();

    setInterval(function() { showAll(); } ,5000);    
});
