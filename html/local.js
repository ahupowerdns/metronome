$(document).ready(function() {
    moment().format();
      
    $.ajaxSetup({ cache: false });

    var comconfig = { url: "http://xs.powerdns.com:8000/", beginTime: -8500 };

    var config1 = { items: [ 
        { name: "pdns.localhost.recursor.questions", legend: "Questions/s" }, 
        { name: "pdns.localhost.recursor.all-outqueries", legend: "All outqueries/s"}]};


    var config2 = { renderer: 'stack', items: [ 
        { name: "pdns.localhost.recursor.answers-slow", legend: "Slow answers/s"},
	{ name: "pdns.localhost.recursor.packetcache-hits", legend: "0ms answers/s"},
        { name: "pdns.localhost.recursor.answers0-1", legend: "<1 ms answers/s"},
        { name: "pdns.localhost.recursor.answers1-10", legend: "1-10ms answers/s"},
        { name: "pdns.localhost.recursor.answers10-100", legend: "10-100ms answers/s"},
        { name: "pdns.localhost.recursor.answers100-1000", legend: "100-1000ms answers/s"}

    ]};

    var config3 ={ renderer: "stack", items: [ 
        { metrics: ["pdns.localhost.recursor.user-msec"], legend: "User CPU%",
	  formula: function(r,d) { return d[0]/10.0; }
	}, 
        { metrics: ["pdns.localhost.recursor.sys-msec"], legend: "System CPU%",
	  formula: function(r,d) { return d[0]/10.0; }
	}]};

    
    var config4 = { items: [ 
        { 
	    metrics: ["pdns.localhost.recursor.packetcache-hits","pdns.localhost.recursor.packetcache-misses"], 
	    legend: "% cache hitrate", 
	    formula: percentalizer
	}]};    


    var config5 ={ renderer: "stack", items: [
        { name: "pdns.localhost.recursor.cache-entries", legend: "Cache entries", kind: "gauge"}, 
        { name: "pdns.localhost.recursor.packetcache-entries", legend: "Packetcache entries", kind: "gauge"}]};

    var config6 = { items: [ 
        { name: "pdns.localhost.auth.udp-queries", legend: "Questions/s" }, 
        { name: "pdns.localhost.auth.udp-answers", legend: "All outqueries/s"}]};


    var config7 = { items: [ 
        { 
	    metrics: ["pdns.localhost.auth.packetcache-hit","pdns.localhost.auth.packetcache-miss"], 
	    legend: "% packet cache hitrate", 
	    formula: percentalizer
	}]};    

    var config8 = { items: [ 
        { 
	    metrics: ["pdns.localhost.auth.query-cache-hit","pdns.localhost.auth.query-cache-miss"], 
	    legend: "% query cache hitrate", 
	    formula: percentalizer
 	}]};    

    function showAll()
    {
	showStuff(comconfig, config1, "#hier1");
	showStuff(comconfig, config2, "#hier2");
	showStuff(comconfig, config3, "#hier3");
	showStuff(comconfig, config4, "#hier4");
	showStuff(comconfig, config5, "#hier5");
	showStuff(comconfig, config6, "#hier6");
	showStuff(comconfig, config7, "#hier7");
	showStuff(comconfig, config8, "#hier8");
    }
    showAll();

    setInterval(showAll() ,5000);    
});
